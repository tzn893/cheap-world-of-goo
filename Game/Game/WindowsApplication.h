#pragma once
#include "IApplication.h"
#include "GraphicModule.h"
#include "Config.h"
#include <Windows.h>
#include <vector>

namespace Game {
	class WindowsApplication : public IApplication{
	public:
		virtual bool initialize() override;
		virtual void tick() override;
		virtual bool isQuit() override;
		virtual void finalize() override;
		virtual void messageBox(const char* title, const char* content) override{
			MessageBoxA(NULL,content,title,MB_OK);
		}

		virtual bool YesNoBox(const char* title,const char* content) override {
			return MessageBoxA(NULL,content,title,MB_YESNO) == IDYES;
		}

		virtual void setTitle(const char* title) override;

		WindowsApplication(int width, int height,IRuntimeModule** runtimeModules,int moduleNum) :
		config(width,height),
		moduleList(moduleNum),winHandle(NULL),
		hinstance(NULL){
			for (int i = 0; i != moduleNum; i++) {
				moduleList[i] = runtimeModules[i];
			}
		}

		HWND getMainWnd() { return winHandle; }

		virtual const Config& getSysConfig() override { return config; }

		void Quit() { quit = true; }
	private:
		bool quit = false;
		HWND winHandle;
		HINSTANCE hinstance;
		
		//GraphicModule* backEnd;
		std::vector<IRuntimeModule*> moduleList;
		Config config;
	};


}