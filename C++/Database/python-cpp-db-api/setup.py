from setuptools import setup, find_packages, Extension
import os
import sys
import glob
import shutil

# Check if we're running from within a CMake build
in_cmake_build = os.environ.get('CMAKE_BINARY_DIR') is not None

# Create required directories if they don't exist
os.makedirs(os.path.join('src', 'python_cpp_db_api'), exist_ok=True)
# Add an __init__.py file to make it a proper package
with open(os.path.join('src', 'python_cpp_db_api', '__init__.py'), 'w') as f:
    f.write('# Python C++ Database API\n')
    f.write('from src.python.api import DatabaseAPI\n')
    f.write('from src.python.connection import DatabaseConnection\n')

# Helper function to find the library file
def find_library_file(library_name):
    # Look in standard locations for the library
    possible_locations = [
        os.path.join('..', 'build', 'lib', 'Release'),
        os.path.join('..', 'build', 'lib', 'Debug'),
        os.path.join('..', 'build', 'lib'),
        os.path.join('..', 'build', 'Release'),
        os.path.join('..', 'build', 'Debug'),
        os.path.join('..', 'build')
    ]
    
    for location in possible_locations:
        lib_path = os.path.join(location, f"{library_name}.lib")
        if os.path.exists(lib_path):
            print(f"Found {library_name}.lib at: {lib_path}")
            return os.path.abspath(os.path.dirname(lib_path))
    
    # If not found, do a recursive search as a last resort
    print(f"Warning: {library_name}.lib not found in standard locations, performing recursive search...")
    search_paths = glob.glob(os.path.join('..', 'build', '**', f"{library_name}.lib"), recursive=True)
    if search_paths:
        print(f"Found {library_name}.lib at: {search_paths[0]}")
        return os.path.abspath(os.path.dirname(search_paths[0]))
    
    print(f"Warning: Could not find {library_name}.lib!")
    return None

# Define the extension module
if in_cmake_build:
    # When built through CMake, the extension is already built
    ext_modules = []
else:
    # For direct pip installs, we need to specify the extension
    # Make sure we use C++17 for all platforms
    extra_compile_args = []
    if sys.platform == 'win32':
        extra_compile_args = ['/std:c++17']  # MSVC
    else:
        extra_compile_args = ['-std=c++17']  # GCC/Clang
    
    # Find the library directory
    lib_dir = find_library_file('database_engine')
    library_dirs = [lib_dir] if lib_dir else ['../build/lib/Release', '../build/lib/Debug', '../build/lib']
    
    ext_modules = [
        Extension(
            'src.python_cpp_db_api._db_module',  # Modified package path
            sources=['bindings/db_module.cpp', 'src/cpp_wrapper/db_bridge.cpp'],
            include_dirs=['src', '../src'],
            library_dirs=library_dirs,
            libraries=['database_engine'],
            extra_compile_args=extra_compile_args,
        )
    ]

# Copy the Python API files to the proper location
python_src_dir = os.path.join('src', 'python')
target_dir = os.path.join('src', 'python_cpp_db_api')
if os.path.exists(python_src_dir):
    print(f"Copying Python API files from {python_src_dir} to {target_dir}")
    for filename in os.listdir(python_src_dir):
        if filename.endswith('.py'):
            src_file = os.path.join(python_src_dir, filename)
            dst_file = os.path.join(target_dir, filename)
            shutil.copy2(src_file, dst_file)
            print(f"Copied {src_file} to {dst_file}")

# Add an empty __init__.py to src directory if it doesn't exist
if not os.path.exists(os.path.join('src', '__init__.py')):
    with open(os.path.join('src', '__init__.py'), 'w') as f:
        f.write('# Package initialization\n')

setup(
    name="python-cpp-db-api",
    version="0.1.0",
    author="Database API Team",
    author_email="example@example.com",
    description="Python API for C++ Database Engine",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/python-cpp-db-api",
    packages=['src.python_cpp_db_api'],  # Explicitly specify the package
    package_dir={"": "."},  # Package is directly in the src directory
    python_requires=">=3.7",
    install_requires=[],
    ext_modules=ext_modules,
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
    ],
    zip_safe=False,
)