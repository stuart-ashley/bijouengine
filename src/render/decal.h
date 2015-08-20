#pragma once

#include "../scripting/scriptObject.h"

#include <string>

class Rect;

namespace render {
	class Texture;

	class Decal: public ScriptObject {
	public:

		/**
		 * constructor
		 *
		 * @param name     name of decal
		 * @param texture  texture for decal
		 * @param flip     flip vertically
		 * @param rect     decal rectangle within texture
		 */
		Decal(const std::string & name,
				const std::shared_ptr<Texture> & texture, bool flip,
				const Rect & rect);

		/**
		 * destructor
		 */
		~Decal();

		bool getFlip() const;

		const std::string & getName() const;

		const Rect & getRect() const;

		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 * get script object factory for Decal
		 *
		 * @param currentDir  current directory of caller
		 *
		 * @return            Decal factory
		 */
		static ScriptObjectPtr getFactory(const std::string & currentDir);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

