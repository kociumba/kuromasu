add_rules("mode.debug", "mode.release", "mode.check")

includes("@builtin/xpack")

package("ktl")
    set_homepage("https://github.com/kociumba/ktl")
    set_description("ktl")
    set_license("MIT")

    set_kind("library", {headeronly = true})

    set_urls("https://github.com/kociumba/ktl.git")

    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)
package_end()

package("_rlimgui")
    set_homepage("https://github.com/raylib-extras/rlImGui")
    set_description("A Raylib integration with DearImGui")
    set_license("zlib")

    add_urls("https://github.com/raylib-extras/rlImGui.git")
    add_versions("2025.11.27", "dc7f97679a024eee8f5f009e77cc311748200415")

    if is_plat("android") then
        add_patches("2025.11.27", "patches/rlImGui.diff", "A1BCC9CDA9734A0700F114172EFC77A59523BD043AD5A8E9CD66191B461DC0BC")
    end

    if is_plat("windows") then
        add_configs("shared", {description = "Build shared library.", default = false, type = "boolean", readonly = true})
    end

    add_deps("raylib")
    add_deps("imgui v1.92.5-docking", {configs = {freetype = true}})

    on_install("!bsd and !iphoneos", function (package)
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            set_languages("c99", "c++17")

            add_requires("raylib")
            add_requires("imgui v1.92.5-docking", {configs = {freetype = true}})

            if is_plat("linux") then
                add_defines("_GLFW_X11", "_GNU_SOURCE")
            end

            if is_plat("android") then
                add_defines("PLATFORM_ANDROID")
            end

            target("rlImGui")
                set_kind("$(kind)")
                add_files("*.cpp")
                add_headerfiles("*.h", "(extras/**.h)")
                add_includedirs(".", {public = true})
                add_packages("raylib", "imgui")
                add_defines("IMGUI_DISABLE_OBSOLETE_FUNCTIONS", "IMGUI_DISABLE_OBSOLETE_KEYIO")
        ]])
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test() {
                rlImGuiBegin();
            }
        ]]}, {includes = {"rlImGui.h"}, configs = {languages = "c++17"}}))
    end)
package_end()

add_requires("ktl 39b236d", "_rlimgui")
set_languages("cxx23")

if is_mode("release") then 
    set_policy("build.optimization.lto", true)
end

target("kuromasu")
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_packages("ktl", "_rlimgui")
    add_extrafiles("assets/**")

    add_rules("utils.bin2obj", {extensions = {".ttf"}})
    add_files("assets/Roboto-Regular.ttf")

    if is_mode("release") and is_plat("windows") then
        add_rules("win.sdk.application") 
    end

    if is_plat("android") then
        add_defines("__ANDROID__", "ASSET_DIR=\"\"")
        add_syslinks("android", "log")
        add_rules("android.native_app", {
            android_sdk_version = "35",
            android_manifest = "android/AndroidManifest.xml",
            android_assets = "assets",
            package_name = "xyz.kociumba.kuromasu",
            keystore = "android/debug.keystore",
            keystore_pass = "android",
            native_app_glue = true,
            logcat_filters = {"kuromasu", "raylib"},
        })
    else 
        add_defines("ASSET_DIR=\"assets/\"")
    end

    if not is_plat("android") then
        after_build(function (target)
            local outdir = target:targetdir()
            local srcdir = path.join(os.projectdir(), "assets")

            os.tryrm(path.join(outdir, "assets"))
            if os.isdir(srcdir) then
                cprint("copying assets -> %s", outdir)
                os.cp(srcdir, outdir)
            end
        end)
    end

xpack("kuromasu")
    set_title("kuromasu")
    set_description("a game of kuromasu")
    set_author("kociumba")

    set_formats("zip", "targz")

    set_bindir("")
    add_targets("kuromasu")
    add_installfiles("assets/**", {prefixdir = "assets"})