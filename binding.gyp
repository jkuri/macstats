{
  "targets": [
    {
      "target_name": "smc",
      "sources": [ "smc/smc.h", "smc/smc.cc" ],
      "link_settings": {
		'libraries': [
		  'IOKit.framework'
		]
      }
    }
  ]
}