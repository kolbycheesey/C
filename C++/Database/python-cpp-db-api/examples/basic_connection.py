"""
Basic example of using the Python API for the C++ database
"""
import os
import sys
import logging

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Add the parent directory to the path so we can import the module
parent_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if parent_dir not in sys.path:
    sys.path.insert(0, parent_dir)

try:
    from src.python.connection import DatabaseConnection
    from src.python.api import DatabaseAPI
except ImportError as e:
    logger.error(f"Error importing database API: {e}")
    logger.info("Make sure you've built the C++ extension module first")
    sys.exit(1)

def main():
    """Main function demonstrating basic database operations"""
    
    # Specify database path
    db_path = os.path.abspath(os.path.join(parent_dir, '..', 'data'))
    logger.info(f"Using database path: {db_path}")
    
    # Create a connection to the database
    try:
        conn = DatabaseConnection(db_path)
        if not conn.connect():
            logger.error("Failed to connect to database")
            return
        logger.info("Successfully connected to database")
    except Exception as e:
        logger.error(f"Error connecting to database: {e}")
        return
    
    # Create an API instance with the connection
    api = DatabaseAPI(conn)
    
    # Perform basic operations
    try:
        # Insert some data
        logger.info("Inserting test data...")
        api.put(1, "Value for key 1")
        api.put(2, "Value for key 2")
        api.put(3, "Value for key 3")
        
        # Retrieve data
        logger.info("Retrieving data...")
        value = api.get(1)
        logger.info(f"Value for key 1: {value}")
        
        # Range query
        logger.info("Performing range query...")
        range_values = api.range_query(1, 3)
        logger.info(f"Range query results: {range_values}")
        
        # Remove data
        logger.info("Removing key 2...")
        api.remove(2)
        
        # Check if removed
        value = api.get(2)
        if value is None:
            logger.info("Key 2 was successfully removed")
        else:
            logger.warning(f"Key 2 still exists with value: {value}")
        
        # Force sync
        logger.info("Syncing database...")
        api.sync()
        
    except Exception as e:
        logger.error(f"Error during database operations: {e}")
    
    # Close the connection when done
    conn.disconnect()
    logger.info("Database connection closed")

if __name__ == "__main__":
    main()