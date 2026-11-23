/*
 * aol-bootstrap: starts the Affinity on Linux loader inside Affinity.
 *
 * affinity-on-linux.exe loads this DLL into Affinity.exe with
 * CreateRemoteThread(LoadLibraryW) once the CLR is up. DllMain starts a
 * thread that asks the running CLR to call AolLoader.EntryPoint.Initialize
 * in aol-loader.dll, which sits next to this DLL.
 *
 * Based on AffinityBootstrap from AffinityPluginLoader by Noah Curoe (MIT).
 * The few CLR hosting interfaces it uses are declared here, so it builds with
 * a plain MinGW-w64 and links only kernel32:
 *
 *   x86_64-w64-mingw32-gcc -O2 -s -shared -o aol-bootstrap.dll aol-bootstrap.c
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

static const GUID CLSID_CLRMetaHost =
    {0x9280188d, 0x0e8e, 0x4867, {0xb3, 0x0c, 0x7f, 0xa8, 0x38, 0x84, 0xe8, 0xde}};
static const GUID IID_ICLRMetaHost =
    {0xd332db9e, 0xb9b3, 0x4125, {0x82, 0x07, 0xa1, 0x48, 0x84, 0xf5, 0x32, 0x16}};
static const GUID IID_ICLRRuntimeInfo =
    {0xbd39d1d2, 0xba2f, 0x486a, {0x89, 0xb0, 0xb4, 0xb0, 0xcb, 0x46, 0x68, 0x91}};
static const GUID CLSID_CLRRuntimeHost =
    {0x90f1a06e, 0x7712, 0x4762, {0x86, 0xb5, 0x7a, 0x5e, 0xba, 0x6b, 0xdb, 0x02}};
static const GUID IID_ICLRRuntimeHost =
    {0x90f1a06c, 0x7712, 0x4762, {0x86, 0xb5, 0x7a, 0x5e, 0xba, 0x6b, 0xdb, 0x02}};

/* Only the slots this file calls are typed; the rest keep their place. */
typedef struct MetaHost MetaHost;
typedef struct RuntimeInfo RuntimeInfo;
typedef struct RuntimeHost RuntimeHost;

struct MetaHost { const struct {
    void *QueryInterface, *AddRef, *Release;
    HRESULT (WINAPI *GetRuntime)(MetaHost *, LPCWSTR, REFIID, void **);
} *vtbl; };

struct RuntimeInfo { const struct {
    void *QueryInterface, *AddRef, *Release;
    void *GetVersionString, *GetRuntimeDirectory, *IsLoaded, *LoadErrorString,
         *LoadLibrary, *GetProcAddress;
    HRESULT (WINAPI *GetInterface)(RuntimeInfo *, REFCLSID, REFIID, void **);
} *vtbl; };

struct RuntimeHost { const struct {
    void *QueryInterface, *AddRef, *Release;
    HRESULT (WINAPI *Start)(RuntimeHost *);
    void *Stop, *SetHostControl, *GetCLRControl, *UnloadAppDomain,
         *ExecuteInAppDomain, *GetCurrentAppDomainId, *ExecuteApplication;
    HRESULT (WINAPI *ExecuteInDefaultAppDomain)(RuntimeHost *, LPCWSTR, LPCWSTR,
                                                LPCWSTR, LPCWSTR, DWORD *);
} *vtbl; };

typedef HRESULT (WINAPI *CLRCreateInstanceFn)(REFCLSID, REFIID, void **);

static void say(const WCHAR *what, HRESULT hr)
{
    WCHAR msg[160];
    _snwprintf(msg, 160, L"aol-bootstrap: %s (0x%08lx)\n", what, (unsigned long)hr);
    msg[159] = 0;
    OutputDebugStringW(msg);
}

static DWORD WINAPI start_loader(void *unused)
{
    WCHAR path[MAX_PATH], *slash;
    HMODULE self = NULL, mscoree;
    CLRCreateInstanceFn create;
    MetaHost *meta = NULL;
    RuntimeInfo *info = NULL;
    RuntimeHost *host = NULL;
    DWORD ret = 0;
    HRESULT hr;

    (void)unused;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&start_loader, &self);
    if (!GetModuleFileNameW(self, path, MAX_PATH) || !(slash = wcsrchr(path, L'\\')))
        return 1;
    if (slash - path + 16 >= MAX_PATH)
        return 1;
    wcscpy(slash + 1, L"aol-loader.dll");

    if (!(mscoree = LoadLibraryW(L"mscoree.dll")) ||
        !(create = (CLRCreateInstanceFn)(void *)GetProcAddress(mscoree, "CLRCreateInstance")))
    {
        say(L"no CLRCreateInstance in mscoree.dll", E_NOINTERFACE);
        return 1;
    }
    if (FAILED(hr = create(&CLSID_CLRMetaHost, &IID_ICLRMetaHost, (void **)&meta)))
        return say(L"CLRCreateInstance failed", hr), 1;
    if (FAILED(hr = meta->vtbl->GetRuntime(meta, L"v4.0.30319", &IID_ICLRRuntimeInfo, (void **)&info)))
        return say(L"no .NET 4 runtime", hr), 1;
    if (FAILED(hr = info->vtbl->GetInterface(info, &CLSID_CLRRuntimeHost, &IID_ICLRRuntimeHost, (void **)&host)))
        return say(L"no runtime host", hr), 1;
    hr = host->vtbl->Start(host); /* S_FALSE: Affinity started it already */
    if (FAILED(hr))
        return say(L"could not start the CLR", hr), 1;

    hr = host->vtbl->ExecuteInDefaultAppDomain(host, path, L"AolLoader.EntryPoint",
                                               L"Initialize", L"", &ret);
    if (FAILED(hr))
        return say(L"aol-loader.dll did not start", hr), 1;
    say(L"loader started", S_OK);
    /* The interfaces stay referenced: the CLR outlives this thread. */
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, void *reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
    {
        HANDLE thread;
        DisableThreadLibraryCalls(dll);
        /* Not from DllMain itself: the CLR takes the loader lock. */
        if ((thread = CreateThread(NULL, 0, start_loader, NULL, 0, NULL)))
            CloseHandle(thread);
        else
            OutputDebugStringW(L"aol-bootstrap: could not start a thread\n");
    }
    return TRUE;
}
