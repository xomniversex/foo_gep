#include <foobar2000.h>

#include "../helpers/win32_misc.h"

#include "resource.h"

#include "loadpic.h"

typedef struct st_logo
{
	unsigned frame;

	IPicture * logos[2];

	POINT ptSize;
	HBITMAP backbuffer;
	HBITMAP frames[2];
} logo_data;


static LRESULT CALLBACK GEPProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HGLOBAL hglob;
	logo_data * gep_gun;

	if (msg == WM_NCCREATE)
	{
		HMODULE hInst = core_api::get_my_instance();
		
		hglob = GlobalAlloc(GMEM_MOVEABLE,sizeof(logo_data));
		gep_gun = (logo_data *) GlobalLock(hglob);

		gep_gun->frame = 0;
		gep_gun->logos[0] = LoadPic(MAKEINTRESOURCE(IDB_LOGO1), _T("JPG"), hInst);
		gep_gun->logos[1] = LoadPic(MAKEINTRESOURCE(IDB_LOGO2), _T("JPG"), hInst);
		
		IPicture_getHandle(gep_gun->logos[0], (OLE_HANDLE *)&gep_gun->frames[0]);
		IPicture_getHandle(gep_gun->logos[1], (OLE_HANDLE *)&gep_gun->frames[1]);

		BITMAP bm;
		GetObject( gep_gun->frames[0], sizeof(bm), &bm );
		gep_gun->ptSize.x = bm.bmWidth;
		gep_gun->ptSize.y = bm.bmHeight;

		BITMAPINFOHEADER bmih;
		bmih.biSize = sizeof(bmih);
		bmih.biWidth = bm.bmWidth;
		bmih.biHeight = bm.bmHeight;
		bmih.biPlanes = bm.bmPlanes;
		bmih.biBitCount = bm.bmBitsPixel;
		bmih.biCompression = BI_RGB;
		bmih.biSizeImage = 0;
		bmih.biXPelsPerMeter = 0;
		bmih.biYPelsPerMeter = 0;
		bmih.biClrUsed = 0;
		bmih.biClrImportant = 0;

		BYTE * gfx;
		gep_gun->backbuffer = CreateDIBSection(0, (const BITMAPINFO *) &bmih, DIB_RGB_COLORS, (VOID **) &gfx, 0, 0);

		SetWindowLong( wnd, GWL_USERDATA, (LONG) hglob );

		SetWindowPos( wnd, 0, 0, 0, gep_gun->ptSize.x, gep_gun->ptSize.y, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);

		SetTimer( wnd, 666, 10, 0 );
	}
	else
	{
		hglob = (HGLOBAL) GetWindowLong( wnd, GWL_USERDATA );
		if (hglob == NULL) return DefWindowProc(wnd, msg, wp, lp);
		gep_gun = (logo_data *) GlobalLock(hglob);
	}
	switch (msg)
	{
	case WM_TIMER:
		if (wp == 666)
		{
			gep_gun->frame++;
			if (gep_gun->frame >= 1000) gep_gun->frame = 0;
			if (!gep_gun->frame ||
				(gep_gun->frame >= 400 && gep_gun->frame <= 500) ||
				(gep_gun->frame >= 900))
			{
				RECT rc;
				GetClientRect( wnd, &rc );
				InvalidateRect( wnd, &rc, FALSE );
			}
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( wnd, &ps );

			HDC hdcMem, hdcBB;
			HBITMAP hbmOld, hbmOldBB;
			BLENDFUNCTION func = { AC_SRC_OVER, 0, 0, 0 };

			ps.rcPaint.right -= ps.rcPaint.left;
			ps.rcPaint.bottom -= ps.rcPaint.top;

			if (gep_gun->frame < 400)
			{
				hdcMem = CreateCompatibleDC( hdc );
				hbmOld = (HBITMAP) SelectObject( hdcMem, gep_gun->frames[0] );

				BitBlt( hdc, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcMem, hbmOld );
				DeleteDC( hdcMem );
			}
			else if (gep_gun->frame < 500)
			{
				hdcBB = CreateCompatibleDC( hdc );
				hbmOldBB = (HBITMAP) SelectObject( hdcBB, gep_gun->backbuffer );

				hdcMem = CreateCompatibleDC( hdc );
				hbmOld = (HBITMAP) SelectObject( hdcMem, gep_gun->frames[0] );

				BitBlt( hdcBB, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcMem, gep_gun->frames[1] );

				func.SourceConstantAlpha = (gep_gun->frame - 400) * 255 / 100;

				AlphaBlend( hdcBB, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, func);

				SelectObject( hdcMem, hbmOld );
				DeleteDC( hdcMem );

				BitBlt( hdc, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcBB,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcBB, hbmOldBB );
				DeleteDC( hdcBB );
			}
			else if (gep_gun->frame < 900)
			{
				hdcMem = CreateCompatibleDC( hdc );
				hbmOld = (HBITMAP) SelectObject( hdcMem, gep_gun->frames[1] );

				BitBlt( hdc, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcMem, hbmOld );
				DeleteDC( hdcMem );
			}
			else if (gep_gun->frame < 1000)
			{
				hdcBB = CreateCompatibleDC( hdc );
				hbmOldBB = (HBITMAP) SelectObject( hdcBB, gep_gun->backbuffer );

				hdcMem = CreateCompatibleDC( hdc );
				hbmOld = (HBITMAP) SelectObject( hdcMem, gep_gun->frames[1] );

				BitBlt( hdcBB, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcMem, gep_gun->frames[0] );

				func.SourceConstantAlpha = (gep_gun->frame - 900) * 255 / 100;

				AlphaBlend( hdcBB, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcMem,
					ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, func);

				SelectObject( hdcMem, hbmOld );
				DeleteDC( hdcMem );

				BitBlt( hdc, ps.rcPaint.left, ps.rcPaint.top,
					ps.rcPaint.right, ps.rcPaint.bottom, hdcBB,
					ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

				SelectObject( hdcBB, hbmOldBB );
				DeleteDC( hdcBB );
			}

			EndPaint( wnd, &ps );
		}
		break;

	/*case WM_LBUTTONDOWN:
		PlaySound(LPCSTR(IDR_COUGH), core_api::get_my_instance(), SND_RESOURCE | SND_ASYNC);
		break;*/

	case WM_DESTROY:
		KillTimer( wnd, 666 );
		SetWindowLong( wnd, GWL_USERDATA, 0 );
		IPicture_Release(gep_gun->logos[0]);
		IPicture_Release(gep_gun->logos[1]);
		DeleteObject(gep_gun->backbuffer);
		GlobalUnlock(hglob);
		GlobalFree(hglob);
		return 0;
	}

	GlobalUnlock(hglob);

	return uDefWindowProc(wnd, msg, wp, lp);
}

registerclass_scope_delayed wnd_class;

static const TCHAR class_name[] = _T( "F8257899-79EC-4860-96E0-6CC6EC80A367" );

bool CreateLogo( HWND parent, int x, int y )
{
	if (!wnd_class.is_registered()) wnd_class.toggle_on( 0, GEPProc, 0, 0, 0, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE+1), class_name, 0 );
	if (wnd_class.is_registered()) return !!CreateWindowEx(0, (const TCHAR *)wnd_class.get_class(), 0, WS_CHILD | WS_VISIBLE, x, y, CW_USEDEFAULT, CW_USEDEFAULT, parent, 0, core_api::get_my_instance(), 0);
	return false;
}
