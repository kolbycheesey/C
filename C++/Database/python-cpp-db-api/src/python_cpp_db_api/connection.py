"""
Database connection module for the C++ Database Engine
"""
import os
import logging
from typing import Optional, Dict, Any

# We'll import the C++ extension module 
try:
    from python_cpp_db_api import _db_module
except ImportError:
    logging.error("Failed to import C++ database module. Make sure it's properly built.")
    _db_module = None

class DatabaseConnection:
    """
    Manages a connection to the C++ database engine
    """
    def __init__(self, db_path: str, config: Optional[Dict[str, Any]] = None):
        """
        Initialize a database connection
        
        Args:
            db_path: Path to the database files
            config: Optional configuration dictionary
        """
        self.db_path = os.path.abspath(db_path)
        self.config = config or {}
        self.connection = None
        self.is_connected = False
        
    def connect(self) -> bool:
        """
        Establish a connection to the database
        
        Returns:
            bool: True if connection was successful, False otherwise
        """
        if not _db_module:
            logging.error("C++ database module not available")
            return False
            
        try:
            self.connection = _db_module.create_connection(self.db_path, self.config)
            self.is_connected = True
            logging.info(f"Successfully connected to database at {self.db_path}")
            return True
        except Exception as e:
            logging.error(f"Failed to connect to database: {e}")
            self.is_connected = False
            return False
    
    def disconnect(self) -> bool:
        """
        Close the database connection
        
        Returns:
            bool: True if disconnection was successful, False otherwise
        """
        if not self.is_connected:
            return True
            
        try:
            _db_module.close_connection(self.connection)
            self.is_connected = False
            self.connection = None
            logging.info("Database connection closed")
            return True
        except Exception as e:
            logging.error(f"Error disconnecting from database: {e}")
            return False
    
    def __enter__(self):
        """Context manager entry"""
        self.connect()
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.disconnect()