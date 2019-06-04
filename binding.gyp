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
	  "src/cxx/thumbs.cc"
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
        "defines": [ "__x86__", "__WIN32__", "OMNI_UNLOADABLE_STUBS" ],
        "include_dirs" : [
          "include/omniORB",
          "include/quantel"
        ],
        "libraries": [
          "-l../lib/omniORB/win32/omniORB4_rt",
          "-l../lib/omniORB/win32/COS4_rt",
          "-l../lib/omniORB/win32/COSDynamic4_rt",
          "-l../lib/omniORB/win32/omniCodeSets4_rt",
          "-l../lib/omniORB/win32/omniConnectionMgmt4_rt",
          "-l../lib/omniORB/win32/omniDynamic4_rt",
          "-l../lib/omniORB/win32/omnisslTP4_rt",
          "-l../lib/omniORB/win32/omnithread_rt"
        ],
        "copies": [
            {
              "destination": "build/Release/",
              "files": [
                "lib/omniORB/win32/omniORB414_vc9_rt.dll",
                "lib/omniORB/win32/COS414_vc9_rt.dll",
                "lib/omniORB/win32/COSDynamic414_vc9_rt.dll",
                "lib/omniORB/win32/omniCodeSets414_vc9_rt.dll",
                "lib/omniORB/win32/omniConnectionMgmt414_vc9_rt.dll",
                "lib/omniORB/win32/omniDynamic414_vc9_rt.dll",
                "lib/omniORB/win32/omnisslTP414_vc9_rt.dll",
                "lib/omniORB/win32/omnithread34_vc9_rt.dll"
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
