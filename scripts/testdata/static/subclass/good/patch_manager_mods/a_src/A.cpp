// A.cpp
g_old = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)wheelProc);
if (g_old && (WNDPROC)GetWindowLongPtrA(g_hwnd, GWLP_WNDPROC) == wheelProc) {
    SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_old);
}
