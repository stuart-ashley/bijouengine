#include "update.h"

#include "draw.h"

#include "core/config.h"
#include "core/loadManager.h"
#include "core/loadManagerRecursive.h"
#include "core/timer.h"

#include "scene/updateState.h"
#include "scene/sceneProgram.h"
#include "scene/sceneProgramManager.h"

#include "scripting/bool.h"
#include "scripting/real.h"
#include "scripting/scriptObject.h"
#include "scripting/scriptException.h"
#include "scripting/string.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <iostream>

struct Update::impl {
	std::atomic<bool> m_ready;
	std::mutex m_lock;
	std::condition_variable m_wait;
	std::atomic<bool> m_stop;
	std::string m_dataDir;

	Draw & m_draw;

	impl(const std::string & dataDir, Draw & draw) :
			m_ready(false), m_stop(false), m_dataDir(dataDir), m_draw(draw) {
	}

	void init() {
		std::unique_lock<std::mutex> locker(m_lock);

		m_ready = true;
		m_wait.notify_all();
	}

	void run() {
		init();

		// load scene
		std::shared_ptr<SceneProgram> sceneProgram;

		const auto & sceneFile = Config::getInstance().getString("sceneFile");
		if (sceneFile != "") {
			auto absoluteFilename = LoadManager::getInstance()->getName(
					m_dataDir, sceneFile);
			if (absoluteFilename.size() > 0) {
				sceneProgram = std::make_shared<SceneProgram>(absoluteFilename);
			} else {
				m_draw.stop();
				m_stop = true;
			}
		}

		auto state = std::make_shared<UpdateState>(m_draw.getWidth(),
				m_draw.getHeight());

		while (m_stop == false) {
			Timer timer;

			state->addEvents(m_draw.getEvents());

			try {
				SceneProgramManager::getInstance().executeLoadedCallbacks();
				auto rg = state->update(sceneProgram);

				m_draw.setRenderGraph(rg);
			} catch (ScriptException & e) {
				std::cerr << e.toString() << std::endl;
				assert(false);

				m_draw.stop();
				break;
			}

			float timeStep = static_cast<float>(std::max(timer.get(), .00001));

			state->setTimeStep(timeStep);
			state->setRenderRate(m_draw.getRenderRate());
		}
	}

	void wait() {
	}
};

Update::Update(const std::string & dataDir, Draw & draw) :
		pimpl(new impl(dataDir, draw)) {
}

Update::~Update() {
}

void Update::run() {
	pimpl->run();
}

void Update::stop() {
	pimpl->m_stop = true;
}

std::thread Update::spawn() {
	assert(pimpl->m_ready == false);

	std::thread thread(&Update::run, this);

	std::unique_lock<std::mutex> locker(pimpl->m_lock);

	while (!pimpl->m_ready) {
		pimpl->m_wait.wait(locker);
	}

	return thread;
}
