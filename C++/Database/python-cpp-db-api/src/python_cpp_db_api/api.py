"""
Python API for interacting with the C++ database engine
"""
import logging
from typing import Optional, Dict, Any, List, Tuple, Union
from .connection import DatabaseConnection

class DatabaseAPI:
    """
    API for performing operations on the C++ database
    """
    def __init__(self, connection: Optional[DatabaseConnection] = None):
        """
        Initialize the database API
        
        Args:
            connection: Optional existing database connection
        """
        self.connection = connection
        self._check_connection()
    
    def _check_connection(self):
        """
        Verify that we have a valid connection
        """
        if not self.connection:
            logging.warning("No database connection provided. Some operations may fail.")
    
    def set_connection(self, connection: DatabaseConnection):
        """
        Set or update the database connection
        
        Args:
            connection: Database connection object
        """
        self.connection = connection
    
    def put(self, key: int, value: str) -> bool:
        """
        Insert or update a key-value pair in the database
        
        Args:
            key: Integer key
            value: String value to store
            
        Returns:
            bool: True if operation was successful
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.put(key, value)
        except Exception as e:
            logging.error(f"Error in put operation: {e}")
            return False
    
    def get(self, key: int) -> Optional[str]:
        """
        Retrieve a value by key
        
        Args:
            key: Integer key to look up
            
        Returns:
            Optional[str]: Value if found, None otherwise
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.get(key)
        except Exception as e:
            logging.error(f"Error in get operation: {e}")
            return None
    
    def remove(self, key: int) -> bool:
        """
        Remove a key-value pair from the database
        
        Args:
            key: Integer key to remove
            
        Returns:
            bool: True if operation was successful
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.remove(key)
        except Exception as e:
            logging.error(f"Error in remove operation: {e}")
            return False
    
    def range_query(self, start_key: int, end_key: int) -> List[Tuple[int, str]]:
        """
        Retrieve all key-value pairs in the given range
        
        Args:
            start_key: Start of key range (inclusive)
            end_key: End of key range (inclusive)
            
        Returns:
            List[Tuple[int, str]]: List of key-value pairs in the range
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.range(start_key, end_key)
        except Exception as e:
            logging.error(f"Error in range query: {e}")
            return []
    
    def execute_query(self, query: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute a query on the database
        
        Args:
            query: SQL-like query string
            params: Optional query parameters
            
        Returns:
            Dict[str, Any]: Query result
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.execute_query(query, params or {})
        except Exception as e:
            logging.error(f"Error executing query: {e}")
            return {"error": str(e), "results": None}
    
    def sync(self) -> bool:
        """
        Force the database to sync its data structures
        
        Returns:
            bool: True if sync was successful
        """
        if not self.connection or not self.connection.is_connected:
            raise ConnectionError("Not connected to database")
            
        try:
            return self.connection.connection.sync()
        except Exception as e:
            logging.error(f"Error syncing database: {e}")
            return False