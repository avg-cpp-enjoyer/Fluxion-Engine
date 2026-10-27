#include "App.hpp"

int FLUXION_STDCALL wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int cmdShow) {
	fluxion::app::App app;

	if (!app.Initialize(1280, 720)) {
		return EXIT_FAILURE;
	}

	return app.Exec(cmdShow);
}