/*
 * Xavi-Gui-Win32.c: GUI Frontend for Xavi using Win32 API
 * Copyright 2015 Vincent Damewood
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <Xavi/Xavi.h>

#include "Menu.h"
#include "About.h"

#define CALCULATOR_INPUT  0x91
#define CALCULATOR_OUTPUT 0x92
#define CALCULATOR_BUTTON 0x93

BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CLOSE:
		EndDialog(hwndDlg, IDOK);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL BuildWindow(HWND hwnd)
{
	HWND Input;
	HWND Output;
	HWND Button;

	Input = CreateWindowEx(
		WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE,
		8, 8, 24, 24, hwnd, (HMENU)CALCULATOR_INPUT,
		GetModuleHandle(NULL), NULL);
	Output = CreateWindowEx(
		0, "STATIC", "0", WS_CHILD | WS_VISIBLE,
		8, 36, 48, 24, hwnd, (HMENU)CALCULATOR_OUTPUT,
		GetModuleHandle(NULL), NULL);
	Button = CreateWindowEx(
		0, "BUTTON", "Calculate", WS_CHILD | WS_VISIBLE,
		8, 36, 100, 28, hwnd, (HMENU)CALCULATOR_BUTTON,
		GetModuleHandle(NULL), NULL);

	return (BOOL) (Input && Output && Button);
}

void ConstrainSize(PRECT Area)
{
	Area->bottom = Area->top + 134;
	if (Area->right - Area->left < 420)
		Area->right = Area->left + 420;

}

void ResizeControls(HWND hwnd)
{
	RECT Area;
	GetClientRect(hwnd, &Area);

	SetWindowPos(GetDlgItem(hwnd, CALCULATOR_INPUT),
		NULL, 8, 8, Area.right - 16, 24, SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, CALCULATOR_OUTPUT),
		NULL, 8, 36, Area.right - 124, 24, SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, CALCULATOR_BUTTON),
		NULL, Area.right - 108, 36, 100, 28, SWP_NOZORDER);
}

void RenderCalculation(HWND hwnd)
{
	int ExpressionSize;
	int ValueSize;
	char *ExpressionString;
	char *ValueString;
	XaviResult Value;

	ExpressionSize = GetWindowTextLength(GetDlgItem(hwnd, CALCULATOR_INPUT)) + 1;
	ExpressionString = (char*)GlobalAlloc(GPTR, ExpressionSize);
	GetDlgItemText(hwnd, CALCULATOR_INPUT, ExpressionString, ExpressionSize);
	
	Value = XaviEvaluate(ExpressionString);

	switch (Value.status)
	{
	case XAVI_RS_INTEGER:
		ValueSize = _snprintf(NULL, 0, "%i", Value.i) + 1;
		ValueString = (char*)GlobalAlloc(GPTR, ValueSize);
		_snprintf(ValueString, ValueSize, "%i", Value.i);
		SetDlgItemText(hwnd, CALCULATOR_OUTPUT, ValueString);
		GlobalFree((HANDLE)ValueString);
		break;
	case XAVI_RS_FLOAT:
		ValueSize = _snprintf(NULL, 0, "%f", Value.f) + 1;
		ValueString = (char*)GlobalAlloc(GPTR, ValueSize);
		_snprintf(ValueString, ValueSize, "%f", Value.f);
		SetDlgItemText(hwnd, CALCULATOR_OUTPUT, ValueString);
		GlobalFree((HANDLE)ValueString);
		break;
	case XAVI_RE_INTERNAL:
		SetDlgItemText(hwnd, CALCULATOR_OUTPUT, "Internal error.");
		break;
	default:
		ValueSize = _snprintf(NULL, 0, "I'm too lazy to report. (%i)", (int)Value.status);
		ValueString = (char*)GlobalAlloc(GPTR, ValueSize);
		_snprintf(ValueString, ValueSize, "I'm too lazy to report. (%i)", (int)Value.status);
		SetDlgItemText(hwnd, CALCULATOR_OUTPUT, ValueString);
		GlobalFree((HANDLE)ValueString);
		break;
	}

	GlobalFree((HANDLE)ExpressionString);
}

LRESULT CALLBACK CalculatorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		if (!BuildWindow(hwnd))
			MessageBox(hwnd, "Error creating controls.", "Startup Error", MB_OK | MB_ICONERROR);
		return 0;
	case WM_SIZING:
		ConstrainSize((PRECT)lParam);
		return 0;
	case WM_SIZE:
		ResizeControls(hwnd);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
        case WM_COMMAND:
        	switch(LOWORD(wParam))
        	{
		case CALCULATOR_BUTTON:
			RenderCalculation(hwnd);
			return 0;
		case HELP_ABOUT:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hwnd, (DLGPROC)AboutDialogProc);
			return 0;
		case CALCULATOR_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		default:
			return 0;
		}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX CalculatorClass;
	HWND Handle;
	MSG Message;

	XaviOpen();

	CalculatorClass.cbSize = sizeof(WNDCLASSEX);
	CalculatorClass.style = 0;
	CalculatorClass.lpfnWndProc = CalculatorWindowProc;
	CalculatorClass.cbClsExtra = 0;
	CalculatorClass.cbWndExtra = 0;
	CalculatorClass.hInstance = hInstance;
	CalculatorClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	CalculatorClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	CalculatorClass.lpszMenuName = MAKEINTRESOURCE(XAVI_MENU);
	CalculatorClass.lpszClassName = "XaviWindow";
	CalculatorClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	CalculatorClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&CalculatorClass))
	{
		MessageBox(NULL, "Xavi was unable to register the calculator window.", "Startup Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}


	if (!(Handle = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"XaviWindow",
		"Xavi",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 420, 134,
		NULL, NULL, hInstance, NULL)))
	{
		MessageBox(NULL, "Xavi was unable to create the calculator window.", "Startup Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(Handle, nCmdShow);
	UpdateWindow(Handle);

	while(GetMessage(&Message, NULL, 0, 0) > 0)
	{
		if (GetDlgCtrlID(Message.hwnd) == CALCULATOR_INPUT
			&& Message.message == WM_KEYDOWN
			&& Message.wParam == VK_RETURN)
		{
			PostMessage(Handle, WM_COMMAND, CALCULATOR_BUTTON, 0);
		}
		else
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	XaviClose();
	return (int) Message.wParam;
}
