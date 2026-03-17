from conan import ConanFile

class CommonConan(ConanFile):
    name = "common"
    version = "1.0"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"
    

    def requirements(self):
        self.requires("gtest/1.16.0")


    def configure(self):
        self.options["gtest"].shared = False
        self.options["gtest"].build_gmock = True
