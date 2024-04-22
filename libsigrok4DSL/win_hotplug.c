

#include "win_hotplug.h"

#ifdef _WIN32

#include <Dbt.h>
#include <SetupAPI.h>
#pragma comment(lib, "SetupAPI.lib")

struct hotplug_ext_info 
{
	HWND window_handle;
	HDEVNOTIFY notify; 
	WNDCLASSEXW class_info;
	hotplug_event_callback hotplug_callback;
};

struct hotplug_ext_info *g_hp_ext_info = NULL;

LRESULT CALLBACK WindowProc( HWND hwnd,	UINT uMsg, WPARAM wParam, LPARAM lParam	)
{
	struct hotplug_ext_info *hp_info = NULL;
	PDEV_BROADCAST_DEVICEINTERFACE broadcast_interface = NULL;
	SP_DEVICE_INTERFACE_DATA interface_data = {0};
	SP_DEVICE_INTERFACE_DETAIL_DATA_W interface_detail_data = {0};
	SP_DEVINFO_DATA devinfo_data = {0};
	HDEVINFO dev_info = NULL;
	WCHAR device_instance_id[2048] = {0};
	CHAR device_instance_id_c[1024] = {0};
	DWORD required_size = 0;
	PCHAR vid = NULL;
	PCHAR pid = NULL;
	DWORD err_code = 0;

	do 
	{
		broadcast_interface = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

		hp_info = (struct hotplug_ext_info *)GetWindowLongPtrW(hwnd, 0xFFFFFFEB);
		if ( !hp_info )
		{
			break;
		}

		if ( uMsg == WM_DEVICECHANGE && broadcast_interface && broadcast_interface->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
		{
			dev_info = SetupDiCreateDeviceInfoList(NULL, NULL);
			if ( !dev_info )
			{
				break;
			}

			interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

			if( !SetupDiOpenDeviceInterfaceW(dev_info, (PWSTR)(lParam + 28), 0, &interface_data) )
			{
				err_code = GetLastError();
				break;
			}

			interface_detail_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

			devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
			devinfo_data.DevInst = NULL;
			devinfo_data.Reserved = 0;

			SetupDiGetDeviceInterfaceDetailW(dev_info, &interface_data, &interface_detail_data, 8, &required_size, &devinfo_data);

			SetupDiGetDeviceInstanceIdW(dev_info, &devinfo_data, device_instance_id, sizeof(device_instance_id), NULL);

			wcstombs(device_instance_id_c, device_instance_id, 1023);

			vid = strstr(device_instance_id_c, "VID_");
			pid = strstr(device_instance_id_c, "PID_");
			if ( !vid || !pid )
			{
				break;
			}

			vid += 4;
			pid += 4;

			vid[4] = '\0';
			pid[4] = '\0';

			if( strtol(vid, NULL, 16) == DS_VENDOR_ID && strtol(pid, NULL, 16) - 1 < 0xFF )
			{
				if ( wParam == DBT_DEVICEARRIVAL )
				{
					if ( hp_info->hotplug_callback )
					{
						hp_info->hotplug_callback(0, 0, 1);
					}
				}
				else if (wParam == DBT_DEVICEREMOVECOMPLETE )
				{
					if ( hp_info->hotplug_callback )
					{
						hp_info->hotplug_callback(0, 0, 2);
					}
				}
			}
		}

	} while (0);

	if ( dev_info )
	{
		SetupDiDestroyDeviceInfoList(dev_info);
		dev_info = NULL;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


int hotplug_ext_init(struct hotplug_ext_info **hp_ext_info)
{
	int ret = -1;
	struct hotplug_ext_info *hp_info = NULL;
	HWND window_handle = NULL;
	PDEV_BROADCAST_DEVICEINTERFACE broadcast_interface = NULL;

	GUID usb_guid = { 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED};

	do 
	{
		hp_info = (struct hotplug_ext_info *)calloc(sizeof(struct hotplug_ext_info), 1);
		if ( !hp_info )
		{
			break;
		}

		hp_info->class_info.cbSize = sizeof(WNDCLASSEXW);
		hp_info->class_info.lpfnWndProc = WindowProc;
		hp_info->class_info.cbWndExtra = 8;
		hp_info->class_info.hInstance = GetModuleHandleW(NULL);
		hp_info->class_info.lpszClassName = L"UsbHotplugClass";
		
		if( !RegisterClassExW(&hp_info->class_info) )
		{
			break;
		}

		window_handle = CreateWindowExW(0, hp_info->class_info.lpszClassName, L"UsbHotplug", 0, 0, 0, 0, 0, 0, 0, GetModuleHandleW(NULL), 0);
		if ( !window_handle )
		{
			break;
		}

		hp_info->window_handle = window_handle;

		SetWindowLongPtrW(window_handle, 0xFFFFFFEB, (LONG_PTR)hp_info);

		broadcast_interface = (PDEV_BROADCAST_DEVICEINTERFACE_W)calloc(sizeof(DEV_BROADCAST_DEVICEINTERFACE_W), 1);
		if ( !broadcast_interface )
		{
			break;
		}

		broadcast_interface->dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE_W);
		broadcast_interface->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		broadcast_interface->dbcc_reserved = 0;
		broadcast_interface->dbcc_name[0] = 0;
		broadcast_interface->dbcc_classguid = usb_guid;

		hp_info->notify = RegisterDeviceNotificationW(window_handle, broadcast_interface, DEVICE_NOTIFY_WINDOW_HANDLE);
		if ( !hp_info->notify )
		{
			break;
		}

		*hp_ext_info = hp_info;

		ret = 0;

	} while (0);

	if ( ret != 0 )
	{
		if ( broadcast_interface )
		{
			free(broadcast_interface);
			broadcast_interface = NULL;
		}
		if ( hp_info )
		{
			free(hp_info);
			hp_info = NULL;
		}
	}

	return ret;
}

int listen_hotplug_ext( struct sr_context *ctx )
{
	int ret = 0;

	if ( g_hp_ext_info == NULL )
	{
		if( hotplug_ext_init(&g_hp_ext_info) == 0 )
		{
			if( ctx ) {
				g_hp_ext_info->hotplug_callback = ctx->hotplug_callback;
			}
		}
		else
		{
			ret = 1;
		}
	}

	return ret;
}

int close_hotplug_ext(struct sr_context *ctx)
{
	if( g_hp_ext_info ) {
		UnregisterDeviceNotification(g_hp_ext_info->notify);
		DestroyWindow(g_hp_ext_info->window_handle);
		UnregisterClassW(g_hp_ext_info->class_info.lpszClassName, g_hp_ext_info->class_info.hInstance);
		
		free(g_hp_ext_info);
		g_hp_ext_info = NULL;
	}
	return 0;
}

int hotplug_wait_timeout_ext(struct sr_context *ctx)
{
	return 0;
}

#endif