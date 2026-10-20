#pragma once

#include "MainWindow.hpp"

namespace fluxion::app {
class App {
public:
	App() = default;
	~App();

	bool Initialize(uint32 width, uint32 height);
	int Exec(int cmdShow);
private:
	MainWindow m_mainWindow;
};
}