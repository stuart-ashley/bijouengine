#pragma once

#include <memory>
#include <vector>

namespace render {
	class Texture;

	class Fbo {
	public:
		Fbo(std::shared_ptr<Texture> const & texture);
		Fbo(std::vector<std::shared_ptr<Texture>> const & textures, int layer);
		Fbo(std::vector<std::shared_ptr<Texture>> const & textures);
		~Fbo();

		bool update(std::shared_ptr<Texture> const & texture);

		bool update(std::vector<std::shared_ptr<Texture>> const & textures);

		static void disable();
	private:
		Fbo();
		Fbo(Fbo const &);
		Fbo & operator =(Fbo const &);

		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

