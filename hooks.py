Import("env", "projenv")
#import os
#from shutil import copyfile
# access to global build environment
print env

# access to project build environment (is used source files in "src" folder)
print projenv

#
# Dump build environment (for debug purpose)
# print env.Dump()
#

#
# Change build flags in runtime
#
env.ProcessUnFlags("-DVECT_TAB_ADDR")
env.Append(CPPDEFINES=("VECT_TAB_ADDR", 0x123456789))

#
# Upload actions
#

def before_upload(source, target, env):
    print "before_upload"
    # do some actions

    # call Node.JS or other script
    env.Execute("node --version")


def after_build(source, target, env):
    print "Copying hex file to target folder"
    env.Execute("mkdir -p target | echo $BUILD_DIR")
    env.Execute("cp $BUILD_DIR/${PROGNAME}.hex target")
    # do some actions

print "Hooks: current build targets", map(str, BUILD_TARGETS)

#env.AddPreAction("upload", before_upload)
#env.AddPostAction("upload", after_upload)

#
# Custom actions when building program/firmware
#

env.AddPostAction("size", after_build)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)

#
# Custom actions for specific files/objects
#

#env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", [callback1, callback2,...])
#env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", callback...)

# custom action before building SPIFFS image. For example, compress HTML, etc.
#env.AddPreAction("$BUILD_DIR/spiffs.bin", callback...)

# custom action for project's main.cpp
#env.AddPostAction("$BUILD_DIR/src/main.cpp.o", callback...)

# Custom HEX from ELF
#env.AddPostAction(
#    "$BUILD_DIR/${PROGNAME}.elf",
#    env.VerboseAction(" ".join([
#        "$OBJCOPY", "-O", "ihex", "-R", ".eeprom",
#        "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}.hex"
#    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
#)