#include <glControls.h>
int main() {
	GLWindow window("testWindow");
	window.use();
	window.createWindow(0, 0, 800, 800);
	int ret = window.initGL();
	if (ret != GLW_SUCCESS)
		fprintf_s(stderr, "init error %d with %d \n", ret, GetLastError());
	gui::GUI::bindWindow(window);
	stdGL glContext;
	printf("Context setup! \n");
	GLImage image;
	printf("Image created! \n");
	image.loadImage("C:\\Users\\stephen\\Pictures\\bored.png");
	printf("Image loaded! \n");
	image.scale(800, 800);
	while (true) {
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		image.draw();
		window.swapBuffers();
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	image.del();
}