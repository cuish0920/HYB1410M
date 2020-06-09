{
 'variables': {
    'opencv_dir%': 'E:/opencv/opencv_sdk',
	'msvc_dir':'C:/Program Files (x86)/Microsoft Visual Studio/2017/Enterprise/VC/Tools/MSVC',
	'Windows_Kits_dir':'C:/Program Files (x86)/Windows Kits',
	'log4api_dir':'F:/ELECTRON9.x/HY-9010_V_B1410M/HY_9010_T/log4api',
	'imperx_sdk_dir':'F:/ELECTRON9.x/HY-9010_V_B1410M/HY_9010_T/Imperx Camera SDK'
  },
  "targets": [
    {
      "target_name": "factoryInterface",
      "sources": [ "factoryInterface.cc","objectFactory.cc","CJsonObject.cpp","SerialUtils.cpp",
	  "cJSON.cpp","EncodeUtils.cpp","xmlOper.cpp","OpencvUtils.cpp","IpxCamera.cpp","threadsafe_queue.h","LogUtils.cpp"],
      "include_dirs":[
         "<!(node -e \"require('nan')\")",
		 "<!(node -e \"require('nan')\")",
		 "<(msvc_dir)/14.16.27023/atlmfc/include",
		 "<(Windows_Kits_dir)/10/Include/10.0.17763.0/um",
		 "<(Windows_Kits_dir)/10/Include/10.0.17763.0/shared",
		 "<(opencv_dir)/include",
		 "<(log4api_dir)/include",
		 "<(imperx_sdk_dir)/inc",
      ],
      "libraries":[],
      "link_settings":{
        "libraries":["-l<(msvc_dir)/14.16.27023/atlmfc/lib/x64/nafxcw.lib",
		"-l<(msvc_dir)/14.16.27023/atlmfc/lib/x64/atls.lib",
		"-l<(opencv_dir)/x64/vc14/lib/opencv_world347.lib",
		"-l<(imperx_sdk_dir)/lib/win64_x64/IpxCameraApi.lib",
		"-l<(log4api_dir)/log4cpp.lib"
		]
      },
      #"cflags!": [ "-fno-exceptions" ],
      #"cflags": [ "-std=c++11" ],
      #"cflags_cc!": [ "-fno-exceptions" ]
    }
  ]
}