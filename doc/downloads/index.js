var jsonData = { "versions": [
                   { "major": 0, "minor": 4, "patch": 1, "day": 20, "month": 5, "year": 2015, "type": 1,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows 7 and later",
                         "files": [
                           { "extension": ".exe", "bitness": 32 }, { "extension": ".zip", "bitness": 32 },
                           { "extension": ".exe", "bitness": 64 }, { "extension": ".zip", "bitness": 64 }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 14.04 LTS (Trusty Tahr) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "Mac OS X 10.7 (Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 0, "patch": 0, "day": 26, "month": 7, "year": 2016, "type": 2,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows 7 and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 14.04 LTS (Trusty Tahr) and later",
                         "files": [
                           { "extension": ".tar.gz" }
                         ]
                       },
                       { "name": "OS X", "supported": "Mac OS X 10.7 (Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ],
                     "changes": [
                       { "change": "<strong>General:</strong> fixed a small problem with the reloading of a modified file potentially resulting in an empty view (see issue <a href=\"https://github.com/opencor/opencor/issues/1052\">#1052</a>). Fixed a small problem with a changed file/dependency resulting in OpenCOR asking to reload it / its owner twice in a row (see issue <a href=\"https://github.com/opencor/opencor/issues/1073\">#1073</a>). Now use specific text interaction flags for a/n information/question/warning/critical/about message box (see issue <a href=\"https://github.com/opencor/opencor/issues/1077\">#1077</a>). Don't define (_)UNICODE globally anymore (see issue <a href=\"https://github.com/opencor/opencor/issues/1080\">#1080</a>)." },
                       { "change": "<strong>File manager:</strong> only check files when OpenCOR is really active (see issue <a href=\"https://github.com/opencor/opencor/issues/1048\">#1048</a>)." },
                       { "change": "<strong>Text-based editing views:</strong> fixed a small problem resulting in the incorrect tracking of the modified state of a file (see issue <a href=\"https://github.com/opencor/opencor/issues/1050\">#1050</a>). Fixed a small problem with no line/column information being shown anymore (see issue <a href=\"https://github.com/opencor/opencor/issues/1053\">#1053</a>)." },
                       { "change": "<strong>MathML viewer:</strong> fixed a small problem with some numbers being rendered with fewer decimal digits than they actually have (see issue <a href=\"https://github.com/opencor/opencor/issues/1054\">#1054</a>)." },
                       { "change": "<strong>Data stores:</strong> fixed a small problem with the number of selected variables being wrongly tracked during an export (see issue <a href=\"https://github.com/opencor/opencor/issues/1076\">#1076</a>)." },
                       { "change": "<strong>Editing views:</strong> fixed a small problem with the reloading of a file resulting in an empty view (see issue <a href=\"https://github.com/opencor/opencor/issues/1091\">#1091</a>)." },
                       { "change": "<strong>Raw SED-ML view:</strong> fixed a small problem with the editor's list not being shown anymore (see issue <a href=\"https://github.com/opencor/opencor/issues/1090\">#1090</a>)." },
                       { "change": "<strong>Single Cell view:</strong> prevent simulation data export from generating some garbage in the case of an unfinished simulation (see issue <a href=\"https://github.com/opencor/opencor/issues/1071\">#1071</a>). The reloading of a SED-ML file only results in the GUI being temporarily disabled when its corresponding view is visible (see issue <a href=\"https://github.com/opencor/opencor/issues/1074\">#1074</a>). Fixed a crash that occurred after running and closing some SED-ML files (see issue <a href=\"https://github.com/opencor/opencor/issues/1083\">#1083</a>)." },
                       { "change": "<strong>Third-party libraries:</strong> upgraded <a href=\"https://riverbankcomputing.com/software/qscintilla/intro\">QScintilla</a> to version 2.9.3 (see issue <a href=\"https://github.com/opencor/opencor/issues/1067\">#1067</a>)." }
                     ]
                   },
                   { "major": 0, "minor": 4, "patch": 0, "day": 29, "month": 4, "year": 2015, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows 7 and later",
                         "files": [
                           { "extension": ".exe", "bitness": 32 }, { "extension": ".zip", "bitness": 32 },
                           { "extension": ".exe", "bitness": 64 }, { "extension": ".zip", "bitness": 64 }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 14.04 LTS (Trusty Tahr) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "Mac OS X 10.7 (Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 3, "patch": 0, "day": 19, "month": 8, "year": 2014, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows XP and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 14.04 LTS (Trusty Tahr) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "Mac OS X 10.7 (Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 2, "patch": 0, "day": 19, "month": 11, "year": 2013, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows XP and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 12.04 LTS (Precise Pangolin) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "Mac OS X 10.7 (Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 1, "patch": 2, "day": 29, "month": 5, "year": 2013, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows XP and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 12.04 LTS (Precise Pangolin) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "OS X 10.8 (Mountain Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 1, "patch": 1, "day": 17, "month": 4, "year": 2013, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows XP and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 12.04 LTS (Precise Pangolin) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "OS X 10.8 (Mountain Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   },
                   { "major": 0, "minor": 1, "patch": 0, "day": 1, "month": 4, "year": 2013, "type": 0,
                     "platforms": [
                       { "name": "Windows", "supported": "Windows XP and later",
                         "files": [
                           { "extension": ".exe" }, { "extension": ".zip" }
                         ]
                       },
                       { "name": "Linux", "supported": "Ubuntu 12.04 LTS (Precise Pangolin) and later",
                         "files": [
                           { "extension": ".tar.gz", "bitness": 32 },
                           { "extension": ".tar.gz", "bitness": 64 }
                         ]
                       },
                       { "name": "OS X", "supported": "OS X 10.8 (Mountain Lion) and later",
                         "files": [
                           { "extension": ".dmg" }, { "extension": ".zip" }
                         ]
                       }
                     ]
                   }
                 ]
               };
