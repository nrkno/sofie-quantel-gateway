{
  "targets": [{
    "target_name" : "quantel_gateway",
    "sources" : [
	  "src/cxx/quantel_gateway.cc",
	  "src/cxx/quentin.cc",
	  "src/cxx/qgw_util.cc",
	  "src/cxx/zone.cc",
	  "src/cxx/clip.cc",
	  "src/cxx/port.cc",
	  "src/cxx/control.cc",
	  "src/cxx/thumbs.cc",
	  "src/cxx/test_server.cc"
	],
    "conditions": [
      ['OS=="win"', {
        "configurations": {
          "Release": {
            "msvs_settings": {
              "VCCLCompilerTool": {
                "RuntimeTypeInfo": "true",
				"ExceptionHandling": 1
              }
            }
          }
        },
        "defines": [ "__x86__", "__WIN32__", "__NT__", "OMNI_UNLOADABLE_STUBS" ],
        "include_dirs" : [
          "include/omniORB",
          "include/quantel"
        ],
        "libraries": [
          "-l../lib/omniORB/win32_x64/omniORB4_rt",
          "-l../lib/omniORB/win32_x64/COS4_rt",
          "-l../lib/omniORB/win32_x64/COSDynamic4_rt",
          "-l../lib/omniORB/win32_x64/omniCodeSets4_rt",
          "-l../lib/omniORB/win32_x64/omniConnectionMgmt4_rt",
          "-l../lib/omniORB/win32_x64/omniDynamic4_rt",
          "-l../lib/omniORB/win32_x64/omnisslTP4_rt",
          "-l../lib/omniORB/win32_x64/omnithread_rt"
        ],
        "copies": [
            {
              "destination": "build/Release/",
              "files": [
                "lib/omniORB/win32_x64/omniORB423_vc15_rt.dll",
                "lib/omniORB/win32_x64/COS423_vc15_rt.dll",
                "lib/omniORB/win32_x64/COSDynamic423_vc15_rt.dll",
                "lib/omniORB/win32_x64/omniCodeSets423_vc15_rt.dll",
                "lib/omniORB/win32_x64/omniConnectionMgmt423_vc15_rt.dll",
                "lib/omniORB/win32_x64/omniDynamic423_vc15_rt.dll",
                "lib/omniORB/win32_x64/omnisslTP423_vc15_rt.dll",
                "lib/omniORB/win32_x64/omnithread41_vc15_rt.dll"
              ]
            }
          ]
    }],
	  ['OS!="win"', {
	    "defines": [
	      "__STDC_CONSTANT_MACROS", "OMNI_UNLOADABLE_STUBS"
	    ],
        "include_dirs" : [
          "include/quantel"
        ],
	    "cflags_cc": [
	      "-std=c++11",
	      "-fexceptions"
	    ],
	    "link_settings": {
	      "libraries": [
	        "-lomniORB4",
			"-lomnithread"
	      ]
	    }
	  }]
	]
  }]
}
