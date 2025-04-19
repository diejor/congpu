from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/11.0.2")
        self.requires("dawn/7069")
        self.requires("slang/2025.6.3")
        self.requires("tracy/0.11.1")

    def build_requirements(self):
        self.test_requires("catch2/3.7.1")

    def configure(self):
        self.settings.compiler.cppstd = "20"
