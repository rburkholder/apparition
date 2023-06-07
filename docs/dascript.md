# daScript installation

pushd 3rd_party/
sudo apt install flex bison
git clone --depth=1 https://github.com/GaijinEntertainment/daScript
cd daScript

$ git diff CMakeLists.txt
diff --git a/CMakeLists.txt b/CMakeLists.txt
index fbeb39c..975b507 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -6,15 +6,15 @@ option(DAS_CLANG_BIND_DISABLED "Disable dasClangBind (libclang bindings, C/C++ p
 option(DAS_LLVM_DISABLED "Disable dasLLVM (llvm bindings)" ON)
 option(DAS_QUIRREL_DISABLED "Disable dasQuirrel (quirrel bindings)" ON)
 option(DAS_HV_DISABLED "Disable dasHV (websokets,http server and client)" ON)
-option(DAS_GLFW_DISABLED "Disable dasGLFW (GLFW window for graphics apps)" OFF)
+option(DAS_GLFW_DISABLED "Disable dasGLFW (GLFW window for graphics apps)" ON)
 option(DAS_IMGUI_DISABLED "Disable dasIMGUI (IMGUI, IMNODES, IMGUI-NODE-EDITOR gui libraries)" ON)
 option(DAS_BGFX_DISABLED "Disable dasBGFX (BGFX graphics API)" ON)
 option(DAS_XBYAK_DISABLED "Disable dasXbyak (XBYAK and ZYDIS, x86 assembly, jit)" ON)
 option(DAS_MINFFT_DISABLED "Disable dasMinfft (Minimal FFT library)" ON)
 option(DAS_SOUND_DISABLED "Disable dasSound (Miniaudio sound library)" ON)
-option(DAS_STDDLG_DISABLED "Disable dasStdDlg (File new,open,save etc dialogs)" OFF)
+option(DAS_STDDLG_DISABLED "Disable dasStdDlg (File new,open,save etc dialogs)" ON)
 option(DAS_STBIMAGE_DISABLED "Disable dasStbImage (StbImage bindings, image loading and saving)" OFF)
-option(DAS_STBTRUETYPE_DISABLED "Disable dasStbTrueType (StbTrueType bindings, ttf rasterization)" OFF)
+option(DAS_STBTRUETYPE_DISABLED "Disable dasStbTrueType (StbTrueType bindings, ttf rasterization)" ON)
 option(DAS_SFML_DISABLED "Disable dasSFML (SFML multimedia library)" ON)
 option(DAS_PUGIXML_DISABLED "Disable dasPUGIXML (xml parsing library)" ON)
 option(DAS_SQLITE_DISABLED "Disable dasSQLITE (sqlite3 library)" ON)

mkdir build
cd build
cmake -G "Unix Makefiles" ..
make daScript
# ignore the following, used in-place for now
#sudo cp -r ../include/daScript /usr/local/include
#sudo cp -R ../include/ska /usr/local/include/daScript/
#sudo cp ./liblibDaScript.a /usr/local/lib/libDaScript.a
popd

