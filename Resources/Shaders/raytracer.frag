#version 330

in vec4 pix_coords;
in vec2 tex_coords;

out vec4 color;

uniform bool u_constFrame;
uniform float u_time;
uniform float u_samples;
uniform float u_sameFrames;
uniform vec2 u_res;
uniform vec2 u_rand1;
uniform vec2 u_rand2;
uniform vec2 u_mouse;
uniform vec3 u_pos;
uniform sampler2D u_backgroudTex;
uniform sampler2D u_previousFrameTex;

uvec4 RANDOM;

#define MAX_DIST 99999.0;
#define MAX_DEPTH 8
#define SPHERE_COUNT 1
#define BOX_COUNT 8
#define N_IN 0.99
#define N_OUT 1.0

struct Material
{
	vec3 emmitance;
    vec3 reflectance;
    float roughness;
    float opacity;
};

Material glass = Material(vec3(0.0), vec3(1.0), 1.0, 0.8);
Material copper = Material(vec3(0.0), vec3(0.6, 0.3, 0.15), 1.0 / 80.0, 0.0);

struct Box
{
    Material material;
    vec3 halfSize;
    vec3 position;
};

struct Sphere
{
    Material material;
    vec3 position;
    float radius;
};

Sphere spheres[SPHERE_COUNT];
Box boxes[BOX_COUNT];

uint TausStep(uint z, int S1, int S2, int S3, uint M)
{
	uint b = (((z << S1) ^ z) >> S2);
	return (((z & M) << S3) ^ b);	
}

uint LCGStep(uint z, uint A, uint C)
{
	return (A * z + C);	
}

float random()
{
	RANDOM.x = TausStep(RANDOM.x, 13, 19, 12, uint(4294967294));
	RANDOM.y = TausStep(RANDOM.y, 2, 25, 4, uint(4294967288));
	RANDOM.z = TausStep(RANDOM.z, 3, 11, 17, uint(4294967280));
	RANDOM.w = LCGStep(RANDOM.w, uint(1664525), uint(1013904223));
	return 2.3283064365387e-10 * float((RANDOM.x ^ RANDOM.y ^ RANDOM.z ^ RANDOM.w));
}

vec2 hash22(vec2 p)
{
	p += u_rand1.x;
	vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.xx + p3.yz) * p3.zy);
}

vec3 randomOnSphere() 
{
	vec3 rand = vec3(random(), random(), random());
	float theta = rand.x * 2.0 * 3.14159265;
	float v = rand.y;
	float phi = acos(2.0 * v - 1.0);
	float r = pow(rand.z, 1.0 / 3.0);
	float x = r * sin(phi) * cos(theta);
	float y = r * sin(phi) * sin(theta);
	float z = r * cos(phi);
	return vec3(x, y, z);
}

vec3 getBackground(vec3 rd)
{
    vec2 uv = vec2(atan(rd.x, rd.y), asin(rd.z) * 2.0);
    uv /= 3.14159265;
    uv = uv * 0.5 + 0.5;
    vec3 col = texture(u_backgroudTex, -uv).rgb;
    //vec3 col = vec3(0.0);
    return col;
}

float FresnelSchlick(float nIn, float nOut, vec3 direction, vec3 normal)
{
    float R0 = ((nOut - nIn) * (nOut - nIn)) / ((nOut + nIn) * (nOut + nIn));
    float fresnel = R0 + (1.0 - R0) * pow((1.0 - abs(dot(direction, normal))), 5.0);
    return fresnel;
}

vec3 IdealRefract(vec3 direction, vec3 normal, float nIn, float nOut)
{
    bool fromOutside = dot(normal, direction) < 0.0;
    float ratio = fromOutside ? nOut / nIn : nIn / nOut;

    vec3 refraction, reflection;

    refraction = fromOutside ? refract(direction, normal, ratio) : -refract(-direction, normal, ratio);
    reflection = reflect(direction, normal);

    return refraction == vec3(0.0) ? reflection : refraction;
}

bool IsRefracted(float rand, vec3 direction, vec3 normal, float opacity, float nIn, float nOut)
{
    float fresnel = FresnelSchlick(nIn, nOut, direction, normal);
    return opacity > rand && fresnel < rand;
}

bool IntersectRaySphere(vec3 origin, vec3 direction, Sphere sphere, out float fraction, out vec3 normal)
{
    vec3 L = origin - sphere.position;
    float a = dot(direction, direction);
    float b = 2.0 * dot(L, direction);
    float c = dot(L, L) - sphere.radius * sphere.radius;
    float D = b * b - 4 * a * c;

    if (D < 0.0) return false;

    float r1 = (-b - sqrt(D)) / (2.0 * a);
    float r2 = (-b + sqrt(D)) / (2.0 * a);
        
    if (r1 > 0.0)
        fraction = r1;
    else if (r2 > 0.0)
        fraction = r2;
    else
        return false;

    normal = normalize(direction * fraction + L);

    return true;
}

bool IntersectRayBox(vec3 origin, vec3 direction, Box box, out float fraction, out vec3 normal)
{
    vec3 rd = direction;
    vec3 ro = (origin - box.position);

    vec3 m = vec3(1.0) / rd; 

    vec3 s = vec3((rd.x < 0.0) ? 1.0 : -1.0,
                  (rd.y < 0.0) ? 1.0 : -1.0,
                  (rd.z < 0.0) ? 1.0 : -1.0);
    vec3 t1 = m * (-ro + s * box.halfSize);
    vec3 t2 = m * (-ro - s * box.halfSize);

    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);

    if (tN > tF || tF < 0.0) return false;

    normal = -sign(rd) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);

    fraction = tN;

    return true;
}

bool CastRay(vec3 rayOrigin, vec3 rayDirection, out float fraction, out vec3 normal, out Material material)
{
    float minDistance = MAX_DIST;

    for (int i = 0; i < SPHERE_COUNT; i++)
    {
        float F;
        vec3 N;
        if (IntersectRaySphere(rayOrigin, rayDirection, spheres[i], F, N) && F < minDistance)
        {
            minDistance = F;
            normal = N;
            material = spheres[i].material;
        }
    }

    for (int i = 0; i < BOX_COUNT; i++)
    {
        float F;
        vec3 N;
        if (IntersectRayBox(rayOrigin, rayDirection, boxes[i], F, N) && F < minDistance)
        {
            minDistance = F;
            normal = N;
            material = boxes[i].material;
        }
    }

    fraction = minDistance;
    return minDistance != MAX_DIST;
}

vec3 TracePath(vec3 rayOrigin, vec3 rayDirection)
{
    vec3 L = vec3(0.0);
    vec3 F = vec3(1.0);
    for (int i = 0; i < MAX_DEPTH; i++)
    {
        float fraction;
        vec3 normal;
        Material material;
        bool hit = CastRay(rayOrigin, rayDirection, fraction, normal, material);
        if (hit)
        {
            vec3 newRayOrigin = rayOrigin + fraction * rayDirection;

            vec3 newRayDirection = randomOnSphere();
            newRayDirection = normalize(newRayDirection * dot(newRayDirection, normal));
            
            float refractRand = random();
            bool refracted = IsRefracted(refractRand, rayDirection, normal, material.opacity, N_IN, N_OUT);
            if (refracted)
            {
                vec3 idealRefraction = IdealRefract(rayDirection, normal, N_IN, N_OUT);
                newRayDirection = normalize(mix(-newRayDirection, idealRefraction, material.roughness));
                newRayOrigin += normal * (dot(newRayDirection, normal) < 0.0 ? -0.8 : 0.8);
            }
            else
            {
                vec3 idealReflection = reflect(rayDirection, normal);
                newRayDirection = normalize(mix(newRayDirection, idealReflection, material.roughness));
                newRayOrigin += normal * 0.8;
            }

            rayDirection = newRayDirection;
            rayOrigin = newRayOrigin;

            L += F * material.emmitance;
            F *= material.reflectance;
        }
        else
        {
            F *= getBackground(rayDirection);
            L += F;
            break;
        }
    }
    return L;
}

mat2 rot(float a)
{
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c);
}

void cornellBox()
{
    boxes[0].position = vec3(0.0, 0.0, 0.0);
    boxes[0].halfSize = vec3(vec2(4.0), 0.2);
    boxes[0].material.opacity = 0.0;
    boxes[0].material.roughness = 0.0;
    boxes[0].material.emmitance = vec3(0.0);
    boxes[0].material.reflectance = vec3(1.0, 1.0, 1.0);

    boxes[1].position = vec3(3.75, 0.0, 4.0);
    boxes[1].halfSize = vec3(0.2, vec2(4.0));
    boxes[1].material.opacity = 0.0;
    boxes[1].material.roughness = 0.0;
    boxes[1].material.emmitance = vec3(0.0);
    boxes[1].material.reflectance = vec3(1.0, 1.0, 1.0);

    boxes[2].position = vec3(0.0, 3.8, 4.0);
    boxes[2].halfSize = vec3(4.0, 0.2, 4.0);
    boxes[2].material.opacity = 0.0;
    boxes[2].material.roughness = 0.0;
    boxes[2].material.emmitance = vec3(0.0);
    boxes[2].material.reflectance = vec3(1.0, 0.0, 0.0);

    boxes[3].position = vec3(0.0, -3.8, 4.0);
    boxes[3].halfSize = vec3(4.0, 0.2, 4.0);
    boxes[3].material.opacity = 0.0;
    boxes[3].material.roughness = 0.0;
    boxes[3].material.emmitance = vec3(0.0);
    boxes[3].material.reflectance = vec3(0.0, 1.0, 0.0);

    boxes[4].position = vec3(0.0, 0.0, 8.0);
    boxes[4].halfSize = vec3(vec2(4.0), 0.2);
    boxes[4].material.opacity = 0.0;
    boxes[4].material.roughness = 0.0;
    boxes[4].material.emmitance = vec3(0.0);
    boxes[4].material.reflectance = vec3(1.0, 1.0, 1.0);

    boxes[5].position = vec3(0.0, 0.0, 7.8);
    boxes[5].halfSize = vec3(vec2(2.0), 0.2);
    boxes[5].material.opacity = 0.0;
    boxes[5].material.roughness = 0.0;
    boxes[5].material.emmitance = vec3(6.0);
    boxes[5].material.reflectance = vec3(1.0, 1.0, 1.0);

    boxes[6].position = vec3(0.0, -1.5, 2.0);
    boxes[6].halfSize = vec3(vec2(1.0), 2.0);
    boxes[6].material.opacity = 0.0;
    boxes[6].material.roughness = 0.0;
    boxes[6].material.emmitance = vec3(0.0);
    boxes[6].material.reflectance = vec3(1.0, 1.0, 1.0);

    boxes[7].position = vec3(0.0, 1.5, 1.0);
    boxes[7].halfSize = vec3(vec2(1.0), 1.0);
    boxes[7].material.opacity = 0.0;
    boxes[7].material.roughness = 0.0;
    boxes[7].material.emmitance = vec3(0.0);
    boxes[7].material.reflectance = vec3(1.0, 1.0, 1.0);

    spheres[0].position = vec3(-2.0, 0.0, 1.5);
    spheres[0].radius = 1.0;
    spheres[0].material = glass;
}

void main()
{
    cornellBox();
    vec2 uv = pix_coords.xy * u_res / u_res.y;

    vec2 uvRes = hash22(uv + 1.0) * u_res + u_res;
	RANDOM.x = uint(u_rand1.x + uvRes.x);
	RANDOM.y = uint(u_rand1.y + uvRes.x);
	RANDOM.z = uint(u_rand2.x + uvRes.y);
	RANDOM.w = uint(u_rand2.y + uvRes.y);

    vec3 rayOrigin = u_pos;
    vec3 rayDirection = normalize(vec3(1.0, uv));

    rayDirection.zx *= rot(-u_mouse.y);
    rayDirection.xy *= rot(u_mouse.x);

    vec3 col = vec3(0.0);

    for(int i = 0; i < u_samples; i++) 
    {
		col += TracePath(rayOrigin, rayDirection);
	}
    col /= u_samples;

    if(u_constFrame)
    {
        vec3 prevColor = texture(u_previousFrameTex, tex_coords).rgb;
        col = mix(prevColor, col, u_sameFrames);
    }

    color = vec4(col, 1.0);
}