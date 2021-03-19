#include "Shader.h"

Shader::Shader(std::string vertexPath, std::string fragmentPath) : m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
{
	m_ShaderID = createProgram();
}

Shader::~Shader()
{
	glDeleteProgram(m_ShaderID);
}

void Shader::use()
{
	glUseProgram(m_ShaderID);
}

void Shader::stopUse()
{
	glUseProgram(0);
}

unsigned int Shader::createProgram()
{
	char messages[1024];

	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vert_path = read_file(m_VertexPath);
	std::string frag_path = read_file(m_FragmentPath);

	const char* vertex_path = vert_path.c_str();
	const char* fragment_path = frag_path.c_str();

	glShaderSource(vertex, 1, &vertex_path, NULL);
	glShaderSource(fragment, 1, &fragment_path, NULL);

	int vertex_compiled;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &vertex_compiled);
	if (!vertex_compiled)
	{
		glGetShaderInfoLog(vertex, 1024, NULL, messages);
		std::cout << messages << std::endl;
	}

	int fragment_compiled;
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &fragment_compiled);
	if (!fragment_compiled)
	{
		glGetShaderInfoLog(fragment, 1024, NULL, messages);
		std::cout << messages << std::endl;
	}


	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	int program_compiled;
	glGetProgramiv(program, GL_LINK_STATUS, &program_compiled);
	if (!program_compiled) {
		glGetProgramInfoLog(program, 512, NULL, messages);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << messages << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return program;
}
