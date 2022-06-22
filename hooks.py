Import("env", "projenv")
import os

# global build environment
#print dir(env)

# project build environment (is used source files in "src" folder)
#print dir(projenv)

env.ProcessUnFlags("-DVECT_TAB_ADDR")
env.Append(CPPDEFINES=("VECT_TAB_ADDR", 0x123456789))


#def enable_device(source, target, env):    
#    print ("Preparing device folder")

def after_build(source, target, env):
    print ("Copying hex file to target folder")
    #if not os.path.isdir("target"):
    #    env.Execute("mkdir target | echo $BUILD_DIR")
    env.Execute("echo ----- copy hex to target folder ---- ")
    env.Execute("cp $BUILD_DIR/${PROGNAME}.hex target")

def after_build2(source, target, env):    
    print ("All done")

print ("Current build targets", map(str, BUILD_TARGETS))

#env.AddPreAction("upload", before_upload)
#env.AddPostAction("upload", after_upload)

def save_hex(*args, **kwargs):
    print("Copying hex output to project directory...")
    target = str(kwargs['target'][0])
    print (kwargs['env'].Environment())
    print (target, 'output.hex')
    print (dir(projenv.Environment()))
    print("Done.")

def save_elf(*args, **kwargs):
    print("Copying elf output to project directory...")
    target = str(kwargs['target'][0])
    copyfile(target, 'output.elf')
    print("Done.")

#env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", save_elf)   #post action for the target hex file
#env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", save_hex)   #post action for the target hex 

env.AddPostAction("size", after_build2)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build2)
#env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", save_hex)
