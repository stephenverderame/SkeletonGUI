#include "glControls.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION
stdShader * stdShader::instance = nullptr;
stdShader::stdShader()
{
	std::string vertexCode = "#version 330 core\r\nlayout(location = 0) in vec2 position;\r\nout vec2 pos;\r\nout vec2 texCoords;\r\nuniform mat4 model;\r\nuniform mat4 projection;\r\nvoid main() {\r\ntexCoords = position;\r\ngl_Position = projection * model * vec4(position, 1.0, 1.0);\r\npos = vec2(gl_Position.x, gl_Position.y);\r\n} ";
	std::string fragmentCode = "#version 330 core \r\nin vec2 pos;\r\nin vec2 texCoords;\r\nout vec4 fragColor;\r\nuniform sampler2D tex;\r\nvoid main() {\r\nvec4 color = texture(tex, texCoords);\r\nif (color.a <= 0.1) discard;\r\nfragColor = color;\r\n}";
	shader = new GLShader();
	shader->loadProgram(vertexCode, fragmentCode);
}
stdShader::~stdShader()
{
	delete shader;
}
stdShader * stdShader::getInstance()
{
	if (instance == nullptr)
		instance = new stdShader();
	return instance;
}

void stdShader::del()
{
	if (instance != nullptr)
		delete instance;
}


GLImage::GLImage()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

}

void GLImage::del()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteTextures(1, &texture);
}

void GLImage::loadImage(const char * path)
{

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char * data = stbi_load(path, &width, &height, &components, 0);
	this->xs = width;
	this->ys = height;
	if (data == NULL) printf("Loaded null ! \n");
	GLenum format;
	switch (components) {
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

void GLImage::translateTo(float x, float y)
{
	this->x = x;
	this->y = y;
}

void GLImage::scale(float xs, float ys)
{
	this->xs = xs;
	this->ys = ys;
}

void GLImage::rotate(float r)
{
	this->r = r;
}

void GLImage::draw()
{
	GLShader * shader = stdShader::getInstance()->glShader();
	shader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glBindVertexArray(vao);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(x, y, 0.0));
	model = glm::scale(model, glm::vec3(xs, ys, 1.0));
	model = glm::rotate(model, r, glm::vec3(0, 0, 1.0));
	shader->setMat4("model", glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 6);

}

stdGL::stdGL()
{
	HWND wind = gui::GUI::useWindow();
	RECT r;
	GetClientRect(wind, &r);
	width = r.right - r.left;
	height = r.bottom - r.top;
	GLShader * shader = stdShader::getInstance()->glShader();
	shader->use();
	glm::mat4 projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.f, 1.f);
	shader->setMat4("projection", glm::value_ptr(projection));
	shader->setInt("tex", 0);
	glViewport(0, 0, width, height);
}

stdGL::~stdGL()
{
	stdShader::del(); 
}
