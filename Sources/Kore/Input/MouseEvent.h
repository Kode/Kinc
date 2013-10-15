#pragma once

namespace Kore {
	class Scene;

	class MouseEvent {
	public:
		MouseEvent();
		MouseEvent(int x, int y);
		void ignore();
		void notIgnore();
		int x() const;
		int y() const;
		int globalX() const;
		int globalY() const;
		bool isIgnored() const;
		void translate(int xdif, int ydif);
		void multiply(float x, float y);
	private:
		int myGlobalX, myGlobalY;
		int currentX, currentY;
		bool ignored;
		friend class Scene;
	};
}
