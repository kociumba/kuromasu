package xyz.kociumba.kuromasu;

import org.libsdl.app.SDLActivity;

public class kuromasu extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[]{
                "SDL3",
                "SDL3_image",
                // "SDL3_mixer",
                // "SDL3_net",
                "SDL3_ttf",
                "kuromasu"
        };
    }
}