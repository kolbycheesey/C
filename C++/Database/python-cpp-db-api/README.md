# python-cpp-db-api

This project provides a Python API for connecting to a C++ database. It allows users to execute queries and manage database connections seamlessly.

## Project Structure

```
python-cpp-db-api
├── src
│   ├── cpp_wrapper         # C++ wrapper for database functionality
│   └── python              # Python API and connection management
├── bindings                 # C++ bindings for Python
├── tests                    # Unit tests for the API
├── examples                 # Example usage of the API
├── setup.py                # Packaging configuration
├── pyproject.toml          # Project configuration
└── CMakeLists.txt          # CMake configuration for building the project
```

## Installation

To install the project, clone the repository and run the following commands:

```bash
# Navigate to the project directory
cd python-cpp-db-api

# Install the required Python packages
pip install -r requirements.txt

# Build the C++ components
mkdir build
cd build
cmake ..
```

### Windows
```bash
# Build using Visual Studio
cmake --build . --config Release
```

### Unix/Linux/MacOS
```bash
# Build using make
make
```

## Usage

### Connecting to the Database

To establish a connection to the database, you can use the following code:

```python
from src.python.connection import DatabaseConnection

db = DatabaseConnection("database_url")
db.connect()
```

### Executing Queries

You can execute queries using the API as follows:

```python
from src.python.api import DatabaseAPI

api = DatabaseAPI()
result = api.execute_query("SELECT * FROM table_name")
print(result)
```

## Examples

Check the `examples` directory for basic and advanced usage examples.

## Testing

To run the tests, use the following command:

```bash
pytest tests/
```

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.