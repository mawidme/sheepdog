package ch.erglim.sheepdog;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    // SDL is linked statically into libmain.so, so that is the only library to load
    @Override
    protected String[] getLibraries() {
        return new String[] { "main" };
    }
}
