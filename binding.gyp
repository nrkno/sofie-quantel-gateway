{
  "targets": [{
    "target_name" : "quantel_gateway",
    "sources" : [ "src/cxx/orbital.cc", "src/cxx/quentin.cc" ],
    "conditions": [
      ['OS=="win"', {
        "configurations": {
          "Release": {
            "msvs_settings": {
              "VCCLCompilerTool": {
                "RuntimeTypeInfo": "true"
              }
            }
          }
        },
        "defines": [ "__x86__", "__WIN32__" ],
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
    }]
  ]
}]
}
