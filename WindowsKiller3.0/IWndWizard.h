#pragma once

#include <Unknwn.h>

#include "IWndPreview.h"
#include "IWndModule.h"
#include "../FileEngine/IFileEngine.h"

// {0AD39BD6-5108-468f-A6E6-05031EDDEDFA}
extern "C" const GUID IID_IViewWizard;
// {1E938831-FDA7-4a8c-91B8-2D306E75F2E6}
extern "C" const GUID CLSID_IWndWizard;

interface RViewWizard
{
	virtual void OnViewWizardEmpty() = 0;
	//If the normal calls, should return true
	virtual bool OnCheckRViewWizard() = 0;
};

interface IWndWizard : public IUnknown
{
	virtual	HWND GetCurrentWindow() = 0;
	virtual bool LaunchViewWizard() = 0;
	virtual	bool ClearViewWizard(void* lpVoid) = 0;
	virtual bool SetIWndModule(IWndModule* pIWndModule) = 0;
	virtual bool SetIWndPreview(IWndPreview* pIWndPreview) = 0;
	virtual bool SetRViewWizard(RViewWizard* pRViewWizard) = 0;
	virtual bool SetIFileEngine(IFileEngine* pIFileEngine) = 0;
	virtual	bool SetBackStage(bool value) = 0;
};
