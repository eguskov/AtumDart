#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main(int argc, char* argv[])
{
  HMODULE module = ::LoadLibrary("AtumExt.dll");
  if (module)
  {
    // BOOL(WINAPI *SetProcessDPIAware)(void) = (BOOL(WINAPI *)(void))(void*)GetProcAddress(hm, "SetProcessDPIAware");
    void* func = GetProcAddress(module, "DartExt_Init");
    printf("%p\n", func);
  }
  else
  {
    DWORD error = GetLastError();
    printf("%d\n", error);
  }

  return 0;
}