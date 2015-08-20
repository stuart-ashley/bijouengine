#pragma once

#include "renderTask.h"

namespace render {

	class ClearScreen: public RenderTask {
	public:

		/**
		 * constructor
		 */
		ClearScreen(const std::string & name);

		/**
		 * destructor
		 */
		~ClearScreen();

		/**
		 * execute task
		 */
		void execute(Canvas & canvas) override;

		/**
		 * @return true if task executed, false otherwise
		 */
		bool hasExecuted() const override;

	private:
		bool m_executed;
	};
}
