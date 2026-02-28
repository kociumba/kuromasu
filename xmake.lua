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

add_requires("imgui v1.92.5-docking", {configs = {freetype = true}})
add_requires("tracy", {configs = {on_demand = true}})
add_requires("ktl 99ca814", "libsdl3_ttf", "libsdl3_image", "libsdl3", "nlohmann_json")
set_languages("cxx23")

if is_plat("android") then
    add_requireconfs("*", {configs = {shared = true}})
end

if is_mode("release") then 
    set_policy("build.optimization.lto", true)
end
set_policy("build.progress_style", "multirow")

target("kuromasu")
    add_files("src/**.cpp")
    add_files("thirdparty/**.cpp")
    add_includedirs("thirdparty")
    add_packages("ktl", "imgui", "libsdl3_ttf", "libsdl3_image", "libsdl3", "tracy", "nlohmann_json")
    add_extrafiles("assets/**")

    add_rules("utils.bin2obj", {extensions = {".ttf"}})
    add_files("assets/Roboto-Regular.ttf")

    
    if is_mode("release") then
        if is_plat("windows") then
            add_rules("win.sdk.application")
        end 

        if is_plat("android") then 
            add_defines("NDEBUG")
        end 
    else 
        add_defines("TRACY_ENABLE", "TRACY_ON_DEMAND")
    end

    if is_plat("android") then
        add_defines("__ANDROID__", "ASSET_DIR=\"\"")
        add_syslinks("android", "log")
        set_kind("shared")
        set_runtimes("c++_shared")
    else 
        add_defines("ASSET_DIR=\"assets/\"")
        set_kind("binary")
        add_headerfiles("src/**.h")
    end

    before_build(function (target)
        local hash = os.iorun("git rev-parse --short HEAD"):trim()
        local commits = os.iorun("git rev-list --count HEAD"):trim()

        
        local build_file = path.join(target:scriptdir(), ".build_cache")
        if not os.isfile(build_file) then
            io.writefile(build_file, "0")
        end

        local n = tonumber(io.readfile(build_file)) or 0
        n = n + 1
        io.writefile(build_file, tostring(n))

        local local_inc = n
        local full_version = string.format("r%s.%s+%s", commits, hash, local_inc)

        target:add("defines", "BUILD_IDENTIFIER=\"" .. full_version .. "\"")
    end)

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