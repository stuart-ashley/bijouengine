#include "draw.h"

#include "core/config.h"
#include "core/frameRate.h"
#include "core/inputEvent.h"
#include "core/vec2.h"

#include "render/renderGraph.h"
#include "render/shaderManager.h"
#include "render/textureManager.h"

#include "scripting/bool.h"
#include "scripting/real.h"
#include "scripting/scriptObject.h"
#include "scripting/string.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <condition_variable>
#include <iostream>

#include <GL/glew.h>

#ifdef WIN32
#include <Windowsx.h>
#else
#include <GL/glx.h>
#include <X11/Xlib.h>
#undef Bool
#undef True
#undef False
#endif

namespace {
	/** mouse events */
	std::vector<ScriptObjectPtr> mousePress = {
			std::make_shared<InputEvent>("down", std::make_shared<String>("left")),
			std::make_shared<InputEvent>("down", std::make_shared<String>("middle")),
			std::make_shared<InputEvent>("down", std::make_shared<String>("right")) };

	std::vector<ScriptObjectPtr> mouseRelease = {
			std::make_shared<InputEvent>("up", std::make_shared<String>("left")),
			std::make_shared<InputEvent>("up", std::make_shared<String>("middle")),
			std::make_shared<InputEvent>("up", std::make_shared<String>("right")),
			std::make_shared<InputEvent>("forward",	std::make_shared<String>("wheel")),
			std::make_shared<InputEvent>("backward", std::make_shared<String>("wheel")) };

	std::string keyAsString(const char * key) {
		std::string k = key;
		std::transform(k.begin(), k.end(), k.begin(), tolower);
		if (k == "period") {
			return ".";
		}
		if (k == "semicolon") {
			return ";";
		}
		return k;
	}
}

#ifdef WIN32
struct Draw::impl {
	bool m_ctrl;
	std::atomic<bool> m_ready;
	std::mutex m_lock;
	std::condition_variable m_wait;
	std::atomic<bool> m_stop;
	unsigned m_width;
	unsigned m_height;
	HINSTANCE m_hInstance;
	HDC m_deviceContext;
	HGLRC m_glContext;
	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> m_outgoingEvents;
	FrameRate m_renderRate;
	std::shared_ptr<render::RenderGraph> m_waitingRenderGraph;
	std::shared_ptr<render::RenderGraph> m_currentRenderGraph;

	impl(HINSTANCE hInstance) :
		m_ctrl(false),
		m_ready(false),
		m_stop(false),
		m_width(Config::getInstance().getInteger("width")),
		m_height(Config::getInstance().getInteger("height")),
		m_hInstance(hInstance),
		m_deviceContext(NULL),
		m_glContext(NULL) {
	}

	std::string convertKey(WPARAM wParam) {
		switch (wParam){
		case VK_DOWN:
			return "down";
		case VK_LEFT:
			return "left";
		case VK_RIGHT:
			return "right";
		case VK_UP:
			return "up";
		case VK_ESCAPE:
			return "escape";
		default:
			return std::string(1, wParam);
		}
		return "";
	}

	LRESULT create(HWND hwnd){
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
			32,                        //Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                        //Number of bits for the depthbuffer
			8,                        //Number of bits for the stencilbuffer
			0,                        //Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat(m_deviceContext, &pfd);
		SetPixelFormat(m_deviceContext, pixelFormat, &pfd);

		m_glContext = wglCreateContext(m_deviceContext);
		wglMakeCurrent(m_deviceContext, m_glContext);

		glewInit();

		return 0;
	}

	LRESULT destroy(HWND hwnd){
		if (m_glContext) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_glContext);
		}
		ReleaseDC(hwnd, m_deviceContext);
		PostQuitMessage(0);
		return 0;
	}

	void displayInit() {
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = wndProc;
		wc.hInstance = m_hInstance;
		wc.lpszClassName = TEXT("oglversionchecksample");
		wc.style = CS_OWNDC;
		if (RegisterClass(&wc)) {
			RECT wr = { 0, 0, m_width, m_height };
			AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
			auto w = wr.right - wr.left;
			auto h = wr.bottom - wr.top;

			CreateWindow(wc.lpszClassName, TEXT("openglversioncheck"), WS_OVERLAPPEDWINDOW |
					WS_VISIBLE, 0, 0, w, h, 0, 0, m_hInstance, this);
		}
	}

	void init() {
		std::unique_lock<std::mutex> locker(m_lock);

		displayInit();

		render::ShaderManager::init();
		render::TextureManager::getInstance().setScreen(m_width, m_height);

		m_ready = true;
		m_wait.notify_all();
	}

	LRESULT keyDown(WPARAM wParam) {
		auto e = std::make_shared<InputEvent>("down",
			std::make_shared<String>(convertKey(wParam)));

		std::lock_guard<std::mutex> locker(m_lock);
		m_outgoingEvents["key"].emplace_back(e);
		return 0;
	}

	LRESULT keyUp(WPARAM wParam) {
		auto e = std::make_shared<InputEvent>("up",
			std::make_shared<String>(convertKey(wParam)));

		std::lock_guard<std::mutex> locker(m_lock);
		m_outgoingEvents["key"].emplace_back(e);
		return 0;
	}

	LRESULT mouseButtonPress(int button) {
		std::lock_guard<std::mutex> locker(m_lock);
		m_outgoingEvents["mouse"].emplace_back(mousePress[button]);
		return 0;
	}

	LRESULT mouseButtonRelease(int button) {
		std::lock_guard<std::mutex> locker(m_lock);
		m_outgoingEvents["mouse"].emplace_back(mouseRelease[button]);
		return 0;
	}

	LRESULT mouseMove(LPARAM lParam) {
		auto x = static_cast<float>(GET_X_LPARAM(lParam)) / m_width;
		auto y = 1.f - static_cast<float>(GET_Y_LPARAM(lParam)) / m_height;
		auto mouseXY = std::make_shared<Vec2>(x, y);

		std::lock_guard<std::mutex> locker(m_lock);

		m_outgoingEvents["mouse"].emplace_back(
			std::make_shared<InputEvent>("position", mouseXY));
		return 0;
	}

	LRESULT mouseWheel(WPARAM wParam) {
		auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		auto msg = zDelta > 0 ? mouseRelease[3] : mouseRelease[4];

		std::lock_guard<std::mutex> locker(m_lock);

		for (int i = 0; i < std::abs(zDelta); i += 120) {
			m_outgoingEvents["mouse"].emplace_back(msg);
		}
		return 0;
	}

	/* return false to stop polling, ie quit, true otherwise */
	bool pollEvents() {
		MSG msg;

		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))	{
			if (msg.message == WM_QUIT) {
				return false;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return true;
	}

	void run() {
		init();

		while (m_stop == false) {
			{
				std::unique_lock<std::mutex> locker(m_lock);
				m_currentRenderGraph = m_waitingRenderGraph;
				m_waitingRenderGraph = nullptr;
			}

			// shader manager update
			render::ShaderManager::getInstance().buildShaders();

			if (m_currentRenderGraph != nullptr) {
				m_renderRate.begin();

				m_currentRenderGraph->execute();

				// swap buffers
				SwapBuffers(m_deviceContext);

				// remove temporary textures
				//texman.scrubCache();

				m_renderRate.update();
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			if (pollEvents() == false) {
				m_stop = true;
			}
		}
	}

	static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		impl * pThis = nullptr;

		if (msg == WM_NCCREATE) {
			pThis = static_cast<impl *>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

			/* get device context */
			pThis->m_deviceContext = GetDC(hwnd);

			SetLastError(0);
			if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis))) {
				if (GetLastError() != 0) {
					return FALSE;
				}
			}
		}
		else {
			pThis = reinterpret_cast<impl *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (pThis != nullptr) {
			switch (msg) {
			case WM_CREATE:
				return  pThis->create(hwnd);
			case WM_DESTROY:
				return  pThis->destroy(hwnd);
			case WM_CLOSE:
				PostQuitMessage(0);
				return 0;
			case WM_KEYDOWN:
				return pThis->keyDown(wParam);
			case WM_KEYUP:
				return pThis->keyUp(wParam);
			case WM_LBUTTONDOWN:
				return pThis->mouseButtonPress(0);
			case WM_MBUTTONDOWN:
				return pThis->mouseButtonPress(1);
			case WM_RBUTTONDOWN:
				return pThis->mouseButtonPress(2);
			case WM_LBUTTONUP:
				return pThis->mouseButtonRelease(0);
			case WM_MBUTTONUP:
				return pThis->mouseButtonRelease(1);
			case WM_RBUTTONUP:
				return pThis->mouseButtonRelease(2);
			case WM_MOUSEWHEEL:
				return pThis->mouseWheel(wParam);
			case WM_MOUSEMOVE:
				return pThis->mouseMove(lParam);
			default:
				break;
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
};

/*
 *
 */
Draw::Draw(HINSTANCE hInstance) :
		pimpl(new impl(hInstance)) {
}
#else
struct Draw::impl {
	bool m_ctrl;
	std::atomic<bool> m_ready;
	std::mutex m_lock;
	std::condition_variable m_wait;
	std::atomic<bool> m_stop;
	unsigned m_width;
	unsigned m_height;
	Display * m_pDisplay;
	Window m_window;
	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> m_outgoingEvents;
	FrameRate m_renderRate;
	std::shared_ptr<render::RenderGraph> m_waitingRenderGraph;
	std::shared_ptr<render::RenderGraph> m_currentRenderGraph;
	Atom m_wmDeleteMessage;

	impl() :
					m_ctrl(false),
					m_ready(false),
					m_stop(false),
					m_width(1024),
					m_height(640),
					m_pDisplay(nullptr),
					m_window(0),
					m_wmDeleteMessage(0) {
	}

	void init() {
		std::unique_lock<std::mutex> locker(m_lock);

		char * name = (char *) "hey";
		displayInit(1, &name);
		glewInit();

		render::ShaderManager::init();
		render::TextureManager::getInstance().setScreen(m_width, m_height);

		m_ready = true;
		m_wait.notify_all();
	}

	void run() {
		init();

		while (m_stop == false) {
			{
				std::unique_lock<std::mutex> locker(m_lock);
				m_currentRenderGraph = m_waitingRenderGraph;
				m_waitingRenderGraph = nullptr;
			}

			// shader manager update
			render::ShaderManager::getInstance().buildShaders();

			if (m_currentRenderGraph != nullptr) {
				m_renderRate.begin();

				m_currentRenderGraph->execute();

				// swap buffers
				glXSwapBuffers(m_pDisplay, m_window);

				// remove temporary textures
				//texman.scrubCache();

				m_renderRate.update();
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			pollInput();
		}
	}

	const char * getKey(unsigned int keycode) {
		int keysyms_per_keycode_return;
		KeySym * keysym = XGetKeyboardMapping(m_pDisplay,
				static_cast<KeyCode>(keycode), 1, &keysyms_per_keycode_return);

		char const * key = XKeysymToString(keysym[0]);

		XFree(keysym);

		return key;
	}

	void pollInput() {
		std::vector<ScriptObjectPtr> mouseEvents;
		std::vector<ScriptObjectPtr> keyEvents;

		while (XPending(m_pDisplay)) {
			XEvent event;
			XNextEvent(m_pDisplay, &event);

			// mouse motion
			if (event.type == MotionNotify) {
				double x = (double) event.xmotion.x / m_width;
				double y = 1 - (double) event.xmotion.y / m_height;
				auto mouseXY = std::make_shared<Vec2>(x, y);
				mouseEvents.emplace_back(
						std::make_shared<InputEvent>("position", mouseXY));
				continue;
			}

			// mouse buttons
			if (event.type == ButtonPress) {
				if (event.xbutton.button <= mousePress.size()) {
					mouseEvents.emplace_back(
							mousePress[event.xbutton.button - 1]);
				}
				continue;
			} else if (event.type == ButtonRelease) {
				if (event.xbutton.button <= mouseRelease.size()) {
					mouseEvents.emplace_back(
							mouseRelease[event.xbutton.button - 1]);
				}
				continue;
			}

			// keyboard
			if (event.type == KeyPress) {
				const char * key = getKey(event.xkey.keycode);

				if (key != nullptr) {
					if (strncmp(key, "Control", 7) == 0) {
						m_ctrl = true;
						key = "ctrl";
					}
					if (m_ctrl && strncmp(key, "q", 1) == 0) {
						m_stop = true;
					}
					auto k = keyAsString(key);
					keyEvents.emplace_back(
							std::make_shared<InputEvent>("down",
									std::make_shared<String>(k)));
				}
			}

			if (event.type == KeyRelease) {
				const char * key = getKey(event.xkey.keycode);

				if (key != nullptr) {
					if (strncmp(key, "Control", 7) == 0) {
						m_ctrl = false;
						key = "ctrl";
					}
					auto k = keyAsString(key);
					keyEvents.emplace_back(
							std::make_shared<InputEvent>("up",
									std::make_shared<String>(k)));
				}
			}

			if (static_cast<unsigned long>(event.xclient.data.l[0])
					== m_wmDeleteMessage) {
				m_stop = true;
			}
		}

		std::lock_guard<std::mutex> locker(m_lock);
		auto & mouse = m_outgoingEvents["mouse"];
		mouse.insert(mouse.end(), mouseEvents.begin(), mouseEvents.end());

		auto & key = m_outgoingEvents["key"];
		key.insert(key.end(), keyEvents.begin(), keyEvents.end());
	}

	/**
	 * initialize display
	 */
	void displayInit(int argc, char * argv[]) {
		// Open a connection to the X server
		m_pDisplay = XOpenDisplay(0);

		if (m_pDisplay == 0) {
			fprintf(stderr, "glxsimple: %s\n", "could not open display");
			exit(1);
		}

		// Make sure OpenGL's GLX extension supported
		int errorBase, eventBase;
		if (!glXQueryExtension(m_pDisplay, &errorBase, &eventBase)) {
			fprintf(stderr, "glxsimple: %s\n",
					"X server has no OpenGL GLX extension");
			exit(1);
		}

		// Find an appropriate visual
		int doubleBufferVisual[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24,
				GLX_DOUBLEBUFFER, None };
		XVisualInfo * visualInfo = glXChooseVisual(m_pDisplay,
				DefaultScreen( m_pDisplay ), doubleBufferVisual);

		if (!visualInfo) {
			fprintf(stderr, "glxsimple: %s\n",
					"no RGB visual with depth buffer");
			exit(1);
		}

		// Create an OpenGL rendering context
		GLXContext glxContext = glXCreateContext(m_pDisplay, visualInfo, 0,
				GL_TRUE);

		if (!glxContext) {
			fprintf(stderr, "glxsimple: %s\n",
					"could not create rendering context");
			exit(1);
		}

		// Create an X colormap since we're probably not using the default visual
		Colormap colorMap = XCreateColormap(m_pDisplay,
				RootWindow( m_pDisplay, visualInfo->screen ) ,
				visualInfo->visual, AllocNone );

		XSetWindowAttributes windowAttributes;
		windowAttributes.colormap = colorMap;
		windowAttributes.border_pixel = 0;
		windowAttributes.event_mask = ExposureMask | VisibilityChangeMask
				| KeyPressMask | KeyReleaseMask | ButtonPressMask
				| ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
				| SubstructureNotifyMask | FocusChangeMask;

		// Create an X window with the selected visual
		m_window = XCreateWindow(m_pDisplay,
				RootWindow( m_pDisplay, visualInfo->screen ) ,
				0, 0, m_width, m_height, 0, visualInfo->depth, InputOutput,visualInfo->visual,
				CWBorderPixel | CWColormap | CWEventMask, &windowAttributes );

		XSetStandardProperties(m_pDisplay, m_window, "GLX Sample", "GLX Sample",
				None, argv, argc, 0);

		// Bind the rendering context to the window
		glXMakeCurrent(m_pDisplay, m_window, glxContext);

		// Request the X window to be displayed on the screen
		XMapWindow(m_pDisplay, m_window);

		// setup window close message
		m_wmDeleteMessage = XInternAtom(m_pDisplay, "WM_DELETE_WINDOW", false);
		XSetWMProtocols(m_pDisplay, m_window, &m_wmDeleteMessage, 1);
	}
};

/*
 *
 */
Draw::Draw() :
		pimpl(new impl()) {
}
#endif

/*
 *
 */
Draw::~Draw() {
}

/*
 *
 */
std::unordered_map<std::string, std::vector<ScriptObjectPtr>> Draw::getEvents() const {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> copy;
	copy.swap(pimpl->m_outgoingEvents);

	return copy;
}

/*
 *
 */
unsigned Draw::getHeight() const {
	return pimpl->m_height;
}

/*
 *
 */
float Draw::getRenderRate() const {
	return pimpl->m_renderRate.getRate();
}

/*
 *
 */
unsigned Draw::getWidth() const {
	return pimpl->m_width;
}

/*
 *
 */
void Draw::run() {
	pimpl->run();
}

/*
 *
 */
void Draw::setRenderGraph(const std::shared_ptr<render::RenderGraph> & rg) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	pimpl->m_waitingRenderGraph = rg;
}

/*
 *
 */
void Draw::stop() {
	pimpl->m_stop = true;
}

/*
 *
 */
std::thread Draw::spawn() {
	assert(pimpl->m_ready == false);

	std::thread thread(&Draw::run, this);

	std::unique_lock<std::mutex> locker(pimpl->m_lock);

	while (!pimpl->m_ready) {
		pimpl->m_wait.wait(locker);
	}

	return thread;
}
