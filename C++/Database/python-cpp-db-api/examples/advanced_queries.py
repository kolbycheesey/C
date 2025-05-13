"""
Advanced example demonstrating query functionality of the Python API
"""
import os
import sys
import logging
import time
import random
from typing import Dict, Any, List

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

def generate_test_data(api: DatabaseAPI, count: int = 1000) -> Dict[int, str]:
    """Generate test data for the database
    
    Args:
        api: Database API instance
        count: Number of records to generate
        
    Returns:
        Dictionary of generated data
    """
    logger.info(f"Generating {count} test records...")
    test_data = {}
    categories = ['electronics', 'clothing', 'books', 'home', 'sports']
    
    for i in range(1, count + 1):
        category = random.choice(categories)
        price = round(random.uniform(10.0, 500.0), 2)
        record = f"{category}|{price}|Product {i}"
        test_data[i] = record
        api.put(i, record)
    
    logger.info(f"Test data generation complete. Inserted {count} records.")
    return test_data

def execute_example_queries(api: DatabaseAPI) -> None:
    """Execute example queries using the query interface
    
    Args:
        api: Database API instance
    """
    # Simple range query
    logger.info("Executing range query for keys 100-110...")
    start_time = time.time()
    results = api.range_query(100, 110)
    elapsed = time.time() - start_time
    
    logger.info(f"Range query returned {len(results)} results in {elapsed:.6f} seconds")
    for key, value in results:
        logger.info(f"  {key}: {value}")
    
    # Parse the results to find items in a specific price range
    logger.info("\nFiltering for electronics items between $50-$100...")
    electronics_items = []
    
    # Get all records
    all_items = api.range_query(1, 1000)
    
    # Parse and filter
    for key, value in all_items:
        parts = value.split('|')
        if len(parts) >= 3:
            category = parts[0]
            try:
                price = float(parts[1])
                if category == 'electronics' and 50.0 <= price <= 100.0:
                    electronics_items.append((key, parts[2], price))
            except ValueError:
                continue
    
    logger.info(f"Found {len(electronics_items)} electronics items between $50-$100")
    for item in electronics_items[:5]:  # Show first 5 results
        logger.info(f"  ID: {item[0]}, Name: {item[1]}, Price: ${item[2]}")
    
    # Example of how SQL-like queries would work (if implemented)
    logger.info("\nExample of how SQL-like queries would work:")
    logger.info("Executing: SELECT * FROM database WHERE category = 'books' AND price < 30")
    
    # This could work via the execute_query method if implemented in the C++ backend
    query = "SELECT * FROM database WHERE category = 'books' AND price < 30"
    params = {"max_price": "30.0", "category": "books"}
    
    try:
        results = api.execute_query(query, params)
        logger.info(f"Query execution status: {results.get('status', 'unknown')}")
    except Exception as e:
        logger.warning(f"Query execution failed: {e}")
        logger.info("Note: SQL-like queries may not be fully implemented in the C++ backend yet")
    
    # Benchmark large range query
    logger.info("\nBenchmarking large range query...")
    start_time = time.time()
    large_range = api.range_query(1, 1000)
    elapsed = time.time() - start_time
    
    logger.info(f"Large range query returned {len(large_range)} results in {elapsed:.6f} seconds")

def main():
    """Main function demonstrating advanced database queries"""
    
    # Specify database path
    db_path = os.path.abspath(os.path.join(parent_dir, '..', 'data'))
    logger.info(f"Using database path: {db_path}")
    
    # Create a connection with configuration options
    config = {
        "cache_size": "64MB",
        "max_open_files": "100",
        "compression": "snappy"
    }
    
    try:
        with DatabaseConnection(db_path, config) as conn:
            logger.info("Connected to database")
            api = DatabaseAPI(conn)
            
            # Generate test data
            generate_test_data(api)
            
            # Force a sync to ensure data is properly stored
            api.sync()
            
            # Execute example queries
            execute_example_queries(api)
            
            logger.info("Advanced query examples completed")
    except Exception as e:
        logger.error(f"Error during database operations: {e}")

if __name__ == "__main__":
    main()