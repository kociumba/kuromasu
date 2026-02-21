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

add_requires("imgui v1.92.5-docking", {configs = {freetype = true, sdl3 = true, sdl3_renderer = true}})
add_requires("ktl 39b236d", "libsdl3_ttf", "libsdl3_image")
set_languages("cxx23")

if is_mode("release") then 
    set_policy("build.optimization.lto", true)
end

target("kuromasu")
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_packages("ktl", "imgui", "libsdl3_ttf", "libsdl3_image")
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
            native_app_glue = false,
            logcat_filters = {"kuromasu", "sdl", "SDL", "dlopen"},
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