#include "pch.h"

#include <Kore/Application.h>
#include <Kore/Files/File.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Shader.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/KeyEvent.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Audio.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Audio/Sound.h>
#include <Kore/Audio/SoundStream.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <stdio.h>

using namespace Kore;

namespace {
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;
	IndexBuffer* indices;

	Texture* titleImage;
	Texture* boardImage;
	Texture* scoreImage;
	
	SoundStream* music;
	
	enum BlockColor {
		Blue, Green, Orange, Purple, Red, Violet, Yellow
	};

	Texture** blockImages;
	
	TextureUnit texUnit;
	VertexStructure structure;

	class Sprite {
	public:
		float x, y, w, h;
		Texture* tex;
		VertexBuffer* vb;

		Sprite() : tex(nullptr) { }

		void init() {
			vb = new VertexBuffer(4, structure);
		}

		//just for demonstration, this is really slow
		void draw() {
			float left = x;
			float right = x + w;
			float top = y;
			float bottom = y + h;
			float* v = vb->lock();
			int i = 0;
			v[i++] = left ; v[i++] = top   ; v[i++] = 0; v[i++] = 0;                                 v[i++] = tex->height / (float)tex->texHeight;
			v[i++] = right; v[i++] = top   ; v[i++] = 0; v[i++] = tex->width / (float)tex->texWidth; v[i++] = tex->height / (float)tex->texHeight;
			v[i++] = left ; v[i++] = bottom; v[i++] = 0; v[i++] = 0;                                 v[i++] = 0;
			v[i++] = right; v[i++] = bottom; v[i++] = 0; v[i++] = tex->width / (float)tex->texWidth; v[i++] = 0;
			vb->unlock();

			vb->set();
			tex->set(texUnit);
			Graphics::drawIndexedVertices();
		}
	};

	Sprite back;

	enum GameState {
		TitleState, InGameState, GameOverState
	};
	GameState state = TitleState;

	class Block;
	Block** blocked;

	class Block : public Sprite {
	public:
		const static int xsize = 12;
		const static int ysize = 23;

		Texture* image;
		vec2i pos;
		vec2i lastpos;
	
		Block(int xx, int yy, Texture* image) : image(image) {
			pos = vec2i(xx, yy);
			lastpos = vec2i(xx, yy);
			Sprite::tex = image;
			init();
		}

		void draw() {
			//272x480
			x = (16 + pos.x() * 16) * 2 / (float)Application::the()->width() - 272.0f / Application::the()->width();
			y = (16 * 4 + pos.y() * 16) * 2 / (float)Application::the()->height() - 480.0f / Application::the()->height();
			w = 16 * 2 / (float)Application::the()->width();
			h = 16 * 2 / (float)Application::the()->height();
			Sprite::draw();
		}

		int getX() {
			return pos.x();
		}
	
		int getY() {
			return pos.y();
		}
	
		vec2i getPos() {
			return pos;
		}
	
		bool right(int i = 1) {
			if (pos.x() + i < xsize && pos.y() < ysize && blocked[pos.y() * xsize + pos.x() + i] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.x() += i;
			return true;
		}
	
		bool left(int i = 1) {
			if (pos.x() - i < xsize && pos.y() < ysize && blocked[pos.y() * xsize + pos.x() - i] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.x() -= i;
			return true;
		}
	
		bool down(int i = 1) {
			if (pos.x() < xsize && pos.y() - i < ysize && blocked[(pos.y() - i) * xsize + pos.x()] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.y() -= i;
			return true;
		}
	
		bool up(int i = 1) {
			if (pos.x() < xsize && pos.y() + i < ysize && blocked[(pos.y() + i) * xsize + pos.x()] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.y() += i;
			return true;
		}
	
		bool rotate(vec2i center) {
			vec2i newpos(center.x() - (pos.y() - center.y()), center.y() + (pos.x() - center.x()));
			if (blocked[newpos.y() * xsize + newpos.x()] != nullptr) return false;
			lastpos = pos;
			pos = newpos;
			return true;
		}
	
		void back() {
			pos = lastpos;
		}
	};

	Sound* rotateSound;
	Sound* lineSound;
	Sound* klackSound;

	class BigBlock {
	public:
		static BigBlock* next;
		static BigBlock* current;
	
		vec2i center;
		Block** blocks;

		BigBlock(int xx, int yy) {
			center = vec2i(xx, yy);
			blocks = new Block*[4];
			for (int i = 0; i < 4; ++i) blocks[i] = nullptr;
		}
	
		void draw() {
			for (int i = 0; i < 4; ++i) blocks[i]->draw();
		}
	
		void right() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->right()) goto retreat;
				++i;
			}
			++center.x();
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}
	
		void left() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->left()) goto retreat;
				++i;
			}
			--center.x();
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}

		bool down() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->down()) goto retreat;
				++i;
			}
			--center.y();
			return true;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
			return false;
		}
	
		Block* getBlock(int b) {
			return blocks[b];
		}
	
		virtual void rotate() {
			Mixer::play(rotateSound);
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->rotate(center)) goto retreat;
				++i;
			}
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}
	
		bool hop() {
			for (int i = 0; i < 4; ++i) {
				blocks[i]->up(8);
				blocks[i]->left(8);
				if (!blocks[i]->down(4)) return false;
			}
			center.x() -= 8;
			center.y() += 4;
			return true;
		}
	};

	class IBlock : public BigBlock {
	public:
		IBlock() : BigBlock(13, 17) {
			Texture* image = blockImages[Red];
			blocks[0] = new Block(13, 18, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 16, image); blocks[3] = new Block(13, 15, image);
		}
	};

	class OBlock : public BigBlock {
	public:
		OBlock() : BigBlock(13, 17) {
			Texture* image = blockImages[Orange];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(12, 17, image);
			blocks[2] = new Block(13, 18, image); blocks[3] = new Block(13, 17, image);
		}
	
		void rotate() override {
		
		}
	};

	class LBlock : public BigBlock {
	public:
		LBlock() : BigBlock(12, 17) {
			Texture* image = blockImages[Blue];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(12, 17, image);
			blocks[2] = new Block(12, 16, image); blocks[3] = new Block(13, 16, image);
		}
	};

	class JBlock : public BigBlock {
	public:
		JBlock() : BigBlock(12, 17) {
			Texture* image = blockImages[Yellow];
			blocks[0] = new Block(13, 18, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 16, image); blocks[3] = new Block(12, 16, image);
		}
	};

	class TBlock : public BigBlock {
	public:
		TBlock() : BigBlock(13, 17) {
			Texture* image = blockImages[Green];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(13, 18, image);
			blocks[2] = new Block(14, 18, image); blocks[3] = new Block(13, 17, image);
		}
	};

	class ZBlock : public BigBlock {
	public:
		ZBlock() : BigBlock(13, 18) {
			Texture* image = blockImages[Purple];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(13, 18, image);
			blocks[2] = new Block(13, 17, image); blocks[3] = new Block(14, 17, image);
		}
	};

	class SBlock : public BigBlock {
	public:
		SBlock() : BigBlock(13, 17) {
			Texture* image = blockImages[Violet];
			blocks[0] = new Block(12, 17, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 18, image); blocks[3] = new Block(14, 18, image);
		}
	};

	bool left = false;
	bool right = false;
	bool lastleft = false;
	bool lastright = false;
	bool down_ = false;
	bool button = false;
	BigBlock* current = nullptr;
	BigBlock* next = nullptr;
	int xcount = 0;
	double lastDownTime = 0;

	bool isBlocked(int x, int y) {
		return blocked[y * Block::xsize + x] != nullptr;
	}

	bool lineBlocked(int y) {
		return isBlocked(1, y) && isBlocked(2, y) && isBlocked(3, y) && isBlocked(4, y) && isBlocked(5, y) &&
			isBlocked(6, y) && isBlocked(7, y) && isBlocked(8, y) && isBlocked(9, y) && isBlocked(10, y);
	}
	
	void check() {
		bool lineDeleted = false;
		for (int i = 0; i < 4; ++i) {
			int y = 1;
			while (y < Block::ysize) {
				if (lineBlocked(y)) {
					lineDeleted = true;
					for (int x = 1; x < Block::xsize - 1; ++x) {
						blocked[y * Block::xsize + x] = nullptr;
					}
					y += 1;
					while (y < Block::ysize) {
						for (int x = 1; x < Block::xsize - 1; ++x) if (blocked[y * Block::xsize + x] != nullptr) {
							blocked[y * Block::xsize + x]->down();
							blocked[(y - 1) * Block::xsize + x] = blocked[y * Block::xsize + x];
							blocked[y * Block::xsize + x] = nullptr;
						}
						++y;
					}
				}
				++y;
			}
		}
		if (lineDeleted) Mixer::play(lineSound);
		else Mixer::play(klackSound);
	}

	BigBlock* createRandomBlock();

	void down() {
		if (!current->down()) {
			down_ = false;
			for (int i = 0; i < 4; ++i) {
				Block* block = current->getBlock(i);
				blocked[block->getY() * Block::xsize + block->getX()] = block;
			}
			current = next;
			next = createRandomBlock();
			check();
			if (!current->hop()) {
				back.tex = scoreImage;
				state = GameOverState;
			}
		}
	}

	void update() {
		Audio::update();
		if (state == InGameState) {
			lastleft = left;
			lastright = right;

			++xcount;
			if (right && !lastright) {
				current->right();
				xcount = 0;
			}
			if (left && !lastleft) {
				current->left();
				xcount = 0;
			}
			if (xcount % 4 == 0) {
				if (right && lastright) current->right();
				else if (left && lastleft) current->left();
			}
			if (button) {
				current->rotate();
				button = false;
			}
			if (down_) down();
			else {
				double time = System::getTime();
				//printf("now %f last %f\n", time, lastDownTime);
				if (time - lastDownTime > 1.0) {
					lastDownTime += 1.0;
					down();
				}
			}
		}

		Graphics::begin();
		Graphics::clear(0);
	
		program->set();
		indices->set();

		back.draw();

		if (state == InGameState) {
			for (int y = 0; y < Block::ysize; ++y) for (int x = 0; x < Block::xsize; ++x) {
				Block* block = blocked[y * Block::xsize + x];
				if (block != nullptr && block->tex != nullptr) {
					block->draw();
				}
			}
			if (current != nullptr) current->draw();
			if (next != nullptr) next->draw();
		}

		Graphics::drawIndexedVertices();

		Graphics::end();
		Graphics::swapBuffers();
	}

	BigBlock* createRandomBlock() {
		switch (Random::get(0, 6)) {
		case 0: return new IBlock();
		case 1: return new LBlock();
		case 2: return new JBlock();
		case 3: return new TBlock();
		case 4: return new ZBlock();
		case 5: return new SBlock();
		case 6: return new OBlock();
		}
		return nullptr;
	}

	void startGame() {
		back.tex = boardImage;

		blocked = new Block*[Block::ysize * Block::xsize];
		for (int y = 0; y < Block::ysize; ++y) for (int x = 0; x < Block::xsize; ++x) blocked[y * Block::xsize + x] = nullptr;
		for (int y = 0; y < Block::ysize; ++y) blocked[y * Block::xsize + 0] = new Block(0, y, nullptr);
		for (int y = 0; y < Block::ysize; ++y) blocked[y * Block::xsize + Block::xsize - 1] = new Block(Block::xsize - 1, y, nullptr);
		for (int x = 0; x < Block::xsize; ++x) blocked[x] = new Block(x, 0, nullptr);
		
		current = createRandomBlock();
		current->hop();
		next = createRandomBlock();

		lastDownTime = System::getTime();
		state = InGameState;
	}

	void keyDown(KeyEvent* event) {
		switch (state) {
		case TitleState:
			startGame();
			break;
		case InGameState:
			switch (event->keycode()) {
			case Kore::Key_Left:
				left = true;
				break;
			case Kore::Key_Right:
				right = true;
				break;
			case Kore::Key_Down:
				down_ = true;
				break;
			case Kore::Key_Up:
				button = true;
				break;
			}
			break;
		}
	}

	void keyUp(KeyEvent* event) {
		switch (state) {
		case InGameState:
			switch (event->keycode()) {
			case Kore::Key_Left:
				left = false;
				break;
			case Kore::Key_Right:
				right = false;
				break;
			case Kore::Key_Down:
				down_ = false;
				break;
			case Kore::Key_Up:
				button = false;
				break;
			}
		}
	}
	
	void mouseUp(MouseEvent event) {
		switch (state) {
			case TitleState:
				startGame();
				break;
		}
	}
}

int kore(int argc, char** argv) {
	Application* app = new Application(argc, argv, 1024, 768, false, "Blocks From Heaven");
	//Sound::init();
	Mixer::init();
	Audio::init();

	app->setCallback(update);

	music = new SoundStream("Sound/blocks.ogg", true);
	rotateSound = new Sound("Sound/rotate.wav");
	lineSound = new Sound("Sound/line.wav");
	klackSound = new Sound("Sound/klack.wav");

	structure.add("pos", Float3VertexData);
	structure.add("tex", Float2VertexData);
	
	DiskFile vs; vs.open("shader.vert", DiskFile::ReadMode);
	DiskFile fs; fs.open("shader.frag", DiskFile::ReadMode);
	vertexShader = new Shader(vs.readAll(), vs.getSize(), VertexShader);
	fragmentShader = new Shader(fs.readAll(), fs.getSize(), FragmentShader);

	program = new Program;
	program->setVertexShader(vertexShader);
	program->setFragmentShader(fragmentShader);
	program->link(structure);

	texUnit = program->getTextureUnit("tex");

	titleImage = new Texture("Graphics/title.png");
	boardImage = new Texture("Graphics/board.png");
	scoreImage = new Texture("Graphics/score.png");

	blockImages = new Texture*[7];
	//Blue, Green, Orange, Purple, Red, Violet, Yellow
	blockImages[Blue] = new Texture("Graphics/block_blue.png");
	blockImages[Green] = new Texture("Graphics/block_green.png");
	blockImages[Orange] = new Texture("Graphics/block_orange.png");
	blockImages[Purple] = new Texture("Graphics/block_purple.png");
	blockImages[Red] = new Texture("Graphics/block_red.png");
	blockImages[Violet] = new Texture("Graphics/block_violet.png");
	blockImages[Yellow] = new Texture("Graphics/block_yellow.png");

	back.tex = titleImage;
	back.x = -titleImage->width / (float)app->width();
	back.y = -titleImage->height / (float)app->height();
	back.w = 2.0f * titleImage->width / (float)app->width();
	back.h = 2.0f * titleImage->height / (float)app->height();
	back.init();
	
	indices = new IndexBuffer(6);
	int* i = indices->lock();
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 1; i[4] = 3; i[5] = 2;
	indices->unlock();

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->ReleaseLeft = mouseUp;

	Mixer::play(music);
	
	lastDownTime = System::getTime();
	app->start();

	return 0;
}
