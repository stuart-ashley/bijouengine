#pragma once

#include "renderTask.h"

namespace render {
	class SetTexture: public RenderTask {
	public:

		SetTexture(const std::string & name,
				const std::shared_ptr<Texture> & texture, const char * data);

		/**
		 * destructor
		 */
		~SetTexture();

		void execute(Canvas & canvas) override;

		bool hasExecuted() const override;

	private:
		const char * data;
		bool executed = false;
	};
}

