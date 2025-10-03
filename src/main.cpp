#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <optional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

static std::optional<std::string> read_file_to_string(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file) {
		std::cerr << "Error reading file: " << filename << "\n";
		return std::nullopt;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string buffer(size, '\0');

	if (!file.read(&buffer[0], size)) {
		std::cerr << "Error reading file: " << filename << "\n";
		return std::nullopt;
	}

	return buffer;
}

static GLuint compile_shader(const char* source, GLenum type) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint did_compile;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &did_compile);
	if (!did_compile) {
		char info_log[256];
		glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
		std::cerr << "Shader compilation failed: " << info_log << "\n";

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

static GLuint compile_program(GLuint vert, GLuint frag) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	glDeleteShader(vert);
	glDeleteShader(frag);

	GLint did_link;
	glGetProgramiv(program, GL_LINK_STATUS, &did_link);
	if (!did_link) {
		char info_log[256];
		glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
		std::cerr << "Shader linking failed: " << info_log << "\n";

		glDeleteProgram(program);
		return 0;
	}

	return program;
}

static GLuint compile_program_from_files(const std::string& vert_filename, const std::string& frag_filename) {
	auto vert_source = read_file_to_string(vert_filename);
	if (!vert_source.has_value()) {
		std::cerr << "Failed to read shader source: " << vert_filename << "\n";
		return 0;
	}

	auto frag_source = read_file_to_string(frag_filename);
	if (!frag_source.has_value()) {
		std::cerr << "Failed to read shader source: " << frag_filename << "\n";
		return 0;
	}

	GLuint vert = compile_shader(vert_source.value().c_str(), GL_VERTEX_SHADER);
	if (!vert) {
		std::cerr << "Failed to compile vertex shader: " << vert_filename << "\n";
		return 0;
	}

	GLuint frag = compile_shader(frag_source.value().c_str(), GL_FRAGMENT_SHADER);
	if (!frag) {
		std::cerr << "Failed to compile fragment shader: " << vert_filename << "\n";
		return 0;
	}

	return compile_program(vert, frag);
}

static GLuint compile_compute_program(const char* filename) {
	auto source = read_file_to_string(filename);
	if (!source.has_value()) {
		std::cerr << "Failed to read shader source: " << filename << "\n";
		return 0;
	}

	GLuint shader = compile_shader(source.value().c_str(), GL_COMPUTE_SHADER);
	if (!shader) {
		std::cerr << "Failed to compile compute shader: " << filename << "\n";
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);

	GLint did_link;
	glGetProgramiv(program, GL_LINK_STATUS, &did_link);
	if (!did_link) {
		char info_log[256];
		glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
		std::cerr << "Shader linking failed: " << info_log << "\n";

		glDeleteProgram(program);
		return 0;
	}

	return program;
}

static GLuint create_output_texture(int width, int height) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

static void glfw_error_callback(int error_code, const char* description) {
	std::cerr << "GLFW: " << description << "\n";
}

template <typename T>
constexpr T div_up(T a, T b) {
	return (a + b - 1) / b;
}

int main() {
	if (!glfwInit()) {
		std::cerr << "glfwInit() failed\n";
		return EXIT_FAILURE;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Quadcraft", NULL, NULL);
	if (!window) {
		std::cerr << "glfwCreateWindow() failed\n";
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cerr << "gladLoadGL() failed\n";
		return EXIT_FAILURE;
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);

	GLuint fullscreen_shader = compile_program_from_files("shaders/fullscreen.vert", "shaders/fullscreen.frag");
	if (!fullscreen_shader) {
		std::cerr << "compile_program_from_files() failed\n";
		return EXIT_FAILURE;
	}

	GLuint pathtracer = compile_compute_program("shaders/pathtracer.comp");
	if (!pathtracer) {
		std::cerr << "compile_compute_program() failed\n";
		return EXIT_FAILURE;
	}

	GLuint output_texture = create_output_texture(mode->width, mode->height);
	if (!output_texture) {
		std::cerr << "output_texture() failed\n";
		return EXIT_FAILURE;
	}

	glBindImageTexture(0, output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	float current_time = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float new_time = (float)glfwGetTime();
		float delta_time = new_time - current_time;
		current_time = new_time;

		glUseProgram(pathtracer);

		int num_groups_x = div_up(mode->width, 8);
		int num_groups_y = div_up(mode->height, 4);
		int num_groups_z = 1;

		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glUseProgram(0);

		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(vao);
		glUseProgram(fullscreen_shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, output_texture);
		glUniform1i(glGetUniformLocation(fullscreen_shader, "u_texture"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUseProgram(0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteProgram(pathtracer);
	glDeleteProgram(fullscreen_shader);
	glDeleteVertexArrays(1, &vao);
	glfwDestroyWindow(window);
	glfwTerminate();
}