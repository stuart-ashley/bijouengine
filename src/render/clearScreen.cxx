#include "clearScreen.h"

#include "textureManager.h"

#include "../core/rect.h"

#include <limits>

#include <GL/glew.h>

using namespace render;

/**
 * constructor
 */
ClearScreen::ClearScreen(const std::string & name) :
				RenderTask(name, TextureManager::getInstance().getScreen(),
						Rect(0, 1, 0, 1), std::numeric_limits<int>::min()),
				m_executed(false) {
}

/**
 * destructor
 */
ClearScreen::~ClearScreen() {
}

/**
 * execute task
 */
OVERRIDE void ClearScreen::execute(Canvas &) {
	auto rect = getViewport();
	glViewport((int) rect.getLeft(), (int) rect.getBottom(),
			(int) rect.getWidth(), (int) rect.getHeight());
	glClearColor(0, 0, 0, 0);
	glClear (GL_COLOR_BUFFER_BIT);

	m_executed = true;
}

/**
 * @return true if task executed, false otherwise
 */
OVERRIDE bool ClearScreen::hasExecuted() const {
	return m_executed;
}
