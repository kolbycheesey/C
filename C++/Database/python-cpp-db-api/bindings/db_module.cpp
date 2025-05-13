#include <Python.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "../src/cpp_wrapper/db_bridge.h"
#include <iostream>

// Forward declarations for all PyObject functions
static PyObject* create_connection_py(PyObject* self, PyObject* args, PyObject* kwargs);
static PyObject* close_connection_py(PyObject* self, PyObject* args);
static PyObject* db_put(PyObject* self, PyObject* args);
static PyObject* db_get(PyObject* self, PyObject* args);
static PyObject* db_remove(PyObject* self, PyObject* args);
static PyObject* db_range(PyObject* self, PyObject* args);
static PyObject* db_sync(PyObject* self, PyObject* args);
static PyObject* db_execute_query(PyObject* self, PyObject* args, PyObject* kwargs);
static PyObject* db_is_connected(PyObject* self, PyObject* args);
static PyObject* db_get_last_error(PyObject* self, PyObject* args);

// Define the methods for the module
static PyMethodDef DBModuleMethods[] = {
    {"create_connection", (PyCFunction)create_connection_py, METH_VARARGS | METH_KEYWORDS, 
     "Create a connection to the database"},
    {"close_connection", close_connection_py, METH_VARARGS, 
     "Close a connection to the database"},
    {"put", db_put, METH_VARARGS, 
     "Put a key-value pair into the database"},
    {"get", db_get, METH_VARARGS, 
     "Get a value by key from the database"},
    {"remove", db_remove, METH_VARARGS, 
     "Remove a key-value pair from the database"},
    {"range", db_range, METH_VARARGS, 
     "Get range of key-value pairs from the database"},
    {"sync", db_sync, METH_VARARGS, 
     "Sync database operations"},
    {"execute_query", (PyCFunction)db_execute_query, METH_VARARGS | METH_KEYWORDS, 
     "Execute a query on the database"},
    {"is_connected", db_is_connected, METH_VARARGS, 
     "Check if connection is active"},
    {"get_last_error", db_get_last_error, METH_VARARGS, 
     "Get the last error message"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// Define the module
static struct PyModuleDef db_module = {
    PyModuleDef_HEAD_INIT,
    "_db_module",    // Module name
    "Python C++ database bindings",  // Module docstring
    -1,              // Size of per-interpreter state or -1
    DBModuleMethods  // Methods
};

// Initialize the module
PyMODINIT_FUNC PyInit__db_module(void) {
    return PyModule_Create(&db_module);
}

// Helper to convert Python dict to C++ unordered_map
static std::unordered_map<std::string, std::string> py_dict_to_map(PyObject* py_dict) {
    std::unordered_map<std::string, std::string> result;
    
    if (!py_dict || !PyDict_Check(py_dict)) {
        return result;
    }
    
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(py_dict, &pos, &key, &value)) {
        PyObject* key_str = PyObject_Str(key);
        PyObject* value_str = PyObject_Str(value);
        
        if (key_str && value_str) {
            result[PyUnicode_AsUTF8(key_str)] = PyUnicode_AsUTF8(value_str);
        }
        
        Py_XDECREF(key_str);
        Py_XDECREF(value_str);
    }
    
    return result;
}

// Helper to convert C++ unordered_map to Python dict
static PyObject* map_to_py_dict(const std::unordered_map<std::string, std::string>& map) {
    PyObject* result = PyDict_New();
    if (!result) {
        return NULL;
    }
    
    for (const auto& pair : map) {
        PyObject* py_value = PyUnicode_FromString(pair.second.c_str());
        if (!py_value) {
            Py_DECREF(result);
            return NULL;
        }
        
        int status = PyDict_SetItemString(result, pair.first.c_str(), py_value);
        Py_DECREF(py_value);
        
        if (status < 0) {
            Py_DECREF(result);
            return NULL;
        }
    }
    
    return result;
}

// Implementation of the database connection creation
static PyObject* create_connection_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    char* db_path;
    PyObject* config_dict = NULL;
    
    // Parse arguments
    static char* kwlist[] = {"db_path", "config", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|O", kwlist, &db_path, &config_dict)) {
        return NULL;
    }
    
    // Convert Python dict to C++ map
    std::unordered_map<std::string, std::string> config = py_dict_to_map(config_dict);
    
    try {
        // Create connection
        auto connection = create_connection(db_path, config);
        if (!connection || !connection->is_connected()) {
            PyErr_SetString(PyExc_ConnectionError, 
                            connection ? connection->get_last_error().c_str() : "Failed to create connection");
            return NULL;
        }
        
        // Return connection as PyCapsule
        return PyCapsule_New(new std::shared_ptr<DatabaseBridge>(connection), "DatabaseConnection", NULL);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Implementation of the database connection closing
static PyObject* close_connection_py(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    
    if (!PyArg_ParseTuple(args, "O", &py_connection)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ shared_ptr from PyCapsule
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Close the connection
    if (!close_connection(*conn_ptr)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to close connection");
        return NULL;
    }
    
    // Delete the heap-allocated shared_ptr
    delete conn_ptr;
    
    Py_RETURN_TRUE;
}

// Put operation
static PyObject* db_put(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    int key;
    char* value;
    
    if (!PyArg_ParseTuple(args, "Ois", &py_connection, &key, &value)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Execute put
    bool result = (*conn_ptr)->put(key, value);
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(PyExc_RuntimeError, (*conn_ptr)->get_last_error().c_str());
        return NULL;
    }
}

// Get operation
static PyObject* db_get(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    int key;
    
    if (!PyArg_ParseTuple(args, "Oi", &py_connection, &key)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Execute get
    auto result = (*conn_ptr)->get(key);
    
    if (result) {
        return PyUnicode_FromString(result->c_str());
    } else {
        Py_RETURN_NONE;
    }
}

// Remove operation
static PyObject* db_remove(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    int key;
    
    if (!PyArg_ParseTuple(args, "Oi", &py_connection, &key)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Execute remove
    bool result = (*conn_ptr)->remove(key);
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(PyExc_RuntimeError, (*conn_ptr)->get_last_error().c_str());
        return NULL;
    }
}

// Range operation
static PyObject* db_range(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    int start_key, end_key;
    
    if (!PyArg_ParseTuple(args, "Oii", &py_connection, &start_key, &end_key)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Execute range
    auto results = (*conn_ptr)->range(start_key, end_key);
    
    // Convert to Python list of tuples
    PyObject* py_results = PyList_New(results.size());
    if (!py_results) {
        return NULL;
    }
    
    for (size_t i = 0; i < results.size(); ++i) {
        PyObject* py_key = PyLong_FromLong(results[i].first);
        PyObject* py_value = PyUnicode_FromString(results[i].second.c_str());
        
        if (!py_key || !py_value) {
            Py_XDECREF(py_key);
            Py_XDECREF(py_value);
            Py_DECREF(py_results);
            return NULL;
        }
        
        PyObject* py_pair = PyTuple_New(2);
        if (!py_pair) {
            Py_DECREF(py_key);
            Py_DECREF(py_value);
            Py_DECREF(py_results);
            return NULL;
        }
        
        PyTuple_SET_ITEM(py_pair, 0, py_key);
        PyTuple_SET_ITEM(py_pair, 1, py_value);
        
        PyList_SET_ITEM(py_results, i, py_pair);
    }
    
    return py_results;
}

// Sync operation
static PyObject* db_sync(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    
    if (!PyArg_ParseTuple(args, "O", &py_connection)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Execute sync
    bool result = (*conn_ptr)->sync();
    
    if (result) {
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(PyExc_RuntimeError, (*conn_ptr)->get_last_error().c_str());
        return NULL;
    }
}

// Execute query operation
static PyObject* db_execute_query(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* py_connection;
    char* query;
    PyObject* params_dict = NULL;
    
    static char* kwlist[] = {"connection", "query", "params", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|O", kwlist, 
                                    &py_connection, &query, &params_dict)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    // Convert params
    std::unordered_map<std::string, std::string> params = py_dict_to_map(params_dict);
    
    // Execute query
    auto result = (*conn_ptr)->execute_query(query, params);
    
    // Convert to Python dict
    return map_to_py_dict(result);
}

// Check connection status
static PyObject* db_is_connected(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    
    if (!PyArg_ParseTuple(args, "O", &py_connection)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        Py_RETURN_FALSE;
    }
    
    if ((*conn_ptr)->is_connected()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

// Get last error
static PyObject* db_get_last_error(PyObject* self, PyObject* args) {
    PyObject* py_connection;
    
    if (!PyArg_ParseTuple(args, "O", &py_connection)) {
        return NULL;
    }
    
    if (!PyCapsule_CheckExact(py_connection)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a DatabaseConnection");
        return NULL;
    }
    
    // Extract C++ object
    auto conn_ptr = static_cast<std::shared_ptr<DatabaseBridge>*>(
        PyCapsule_GetPointer(py_connection, "DatabaseConnection")
    );
    
    if (!conn_ptr || !*conn_ptr) {
        PyErr_SetString(PyExc_ValueError, "Invalid DatabaseConnection");
        return NULL;
    }
    
    return PyUnicode_FromString((*conn_ptr)->get_last_error().c_str());
}