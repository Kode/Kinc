#pragma once

#include <Kt/Exception.h>

namespace Kt {
	class HResultException : public Exception {
	public:
		HResultException(long result);
		virtual const char* what() override;
	private:
		long result;
	};

	void affirm(long result);
}