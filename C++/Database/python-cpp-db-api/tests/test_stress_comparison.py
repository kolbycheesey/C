"""
Stress test comparing the performance of our database against MongoDB
"""
import unittest
import time
import random
import uuid
import os
import sys
import logging
from typing import Dict, List, Any, Tuple
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Add the parent directory to the Python path to enable imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# Try importing pymongo, but handle the case where it's not installed
try:
    import pymongo
    MONGODB_AVAILABLE = True
except ImportError:
    MONGODB_AVAILABLE = False
    print("MongoDB driver (pymongo) not available. MongoDB tests will be skipped.")

# Import our database API - updated import paths
try:
    # First try to import from the installed package
    from src.python_cpp_db_api.api import DatabaseAPI
    from src.python_cpp_db_api.connection import DatabaseConnection
except ImportError:
    # Fall back to direct import from source
    try:
        from src.python.api import DatabaseAPI
        from src.python.connection import DatabaseConnection
    except ImportError:
        print("Could not import database modules. Make sure the package is installed or paths are correct.")
        DatabaseAPI = None
        DatabaseConnection = None

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class StressTestComparison(unittest.TestCase):
    """Stress test class that compares our database against MongoDB"""
    
    # Test dataset sizes
    DATASET_SIZES = [100, 1000, 10000]
    
    # Number of read operations for each test
    READ_OPERATIONS = 1000
    
    # Number of test iterations to run (for averaging results)
    TEST_ITERATIONS = 3
    
    @classmethod
    def setUpClass(cls):
        """Set up the test environment"""
        # Create our database connection
        db_path = os.path.join(os.path.dirname(__file__), '..', '..', 'build', 'data', 'stress_test')
        cls.our_db = None
        
        try:
            connection = DatabaseConnection(db_path)
            cls.our_db = DatabaseAPI(connection)
            logger.info("Our database connection established")
        except Exception as e:
            logger.error(f"Error connecting to our database: {e}")
        
        # Set up MongoDB connection if available
        cls.mongodb = None
        if MONGODB_AVAILABLE:
            try:
                client = pymongo.MongoClient("mongodb://localhost:27017/")
                cls.mongodb = client.stress_test_db
                # Drop the collection if it exists to start fresh
                if 'stress_test' in cls.mongodb.list_collection_names():
                    cls.mongodb.drop_collection('stress_test')
                cls.mongodb_collection = cls.mongodb.stress_test
                logger.info("MongoDB connection established")
            except Exception as e:
                logger.error(f"Error connecting to MongoDB: {e}")
                cls.mongodb = None

    @classmethod
    def tearDownClass(cls):
        """Clean up after tests"""
        # Close our database connection
        if cls.our_db is not None and cls.our_db.connection is not None:
            try:
                cls.our_db.connection.disconnect()
                logger.info("Our database connection closed")
            except Exception as e:
                logger.error(f"Error closing our database connection: {e}")
        
        # Clean up MongoDB
        if cls.mongodb is not None and MONGODB_AVAILABLE:
            try:
                # Drop the test collection
                cls.mongodb.drop_collection('stress_test')
                logger.info("MongoDB test collection dropped")
            except Exception as e:
                logger.error(f"Error cleaning up MongoDB: {e}")

    def generate_test_data(self, size: int) -> List[Dict[str, Any]]:
        """Generate test data of the specified size
        
        Args:
            size: Number of records to generate
            
        Returns:
            List of dictionaries representing test records
        """
        data = []
        logger.info(f"Generating {size} test records...")
        
        for i in range(size):
            record = {
                'id': i,
                'uuid': str(uuid.uuid4()),
                'name': f"Test Record {i}",
                'value': random.random() * 1000,
                'timestamp': time.time(),
                'tags': [
                    random.choice(['tag1', 'tag2', 'tag3', 'tag4', 'tag5'])
                    for _ in range(random.randint(1, 5))
                ],
                'active': random.choice([True, False]),
                'metadata': {
                    'source': 'stress_test',
                    'priority': random.randint(1, 5),
                    'category': random.choice(['A', 'B', 'C', 'D'])
                }
            }
            data.append(record)
            
        return data

    def test_write_performance(self):
        """Test the write performance of both databases"""
        results = {'dataset_size': [], 'our_db_time': [], 'mongodb_time': []}
        
        # Skip the test if either database is not available
        if self.our_db is None or (self.mongodb is None and MONGODB_AVAILABLE):
            self.skipTest("One or both databases are not available")
            
        for size in self.DATASET_SIZES:
            our_db_times = []
            mongodb_times = []
            
            for iteration in range(self.TEST_ITERATIONS):
                # Generate test data
                test_data = self.generate_test_data(size)
                
                # Test our database write performance
                start_time = time.time()
                for record in test_data:
                    # Convert record to string representation for our key-value store
                    import json
                    self.our_db.put(record['id'], json.dumps(record))
                our_db_time = time.time() - start_time
                our_db_times.append(our_db_time)
                
                # Sync to ensure data is persisted
                self.our_db.sync()
                
                # Test MongoDB write performance if available
                if self.mongodb is not None and MONGODB_AVAILABLE:
                    start_time = time.time()
                    self.mongodb_collection.insert_many(test_data)
                    mongodb_time = time.time() - start_time
                    mongodb_times.append(mongodb_time)
                    
                    # Clear collection for next iteration
                    self.mongodb_collection.delete_many({})
                else:
                    mongodb_times.append(None)
                
                logger.info(f"Dataset size {size}, iteration {iteration+1}: "
                           f"Our DB: {our_db_time:.4f}s, "
                           f"MongoDB: {mongodb_times[-1]:.4f if mongodb_times[-1] else 'N/A'}s")
            
            # Calculate average times
            avg_our_db = sum(our_db_times) / len(our_db_times)
            results['dataset_size'].append(size)
            results['our_db_time'].append(avg_our_db)
            
            if self.mongodb is not None and MONGODB_AVAILABLE:
                avg_mongodb = sum(mongodb_times) / len(mongodb_times)
                results['mongodb_time'].append(avg_mongodb)
            else:
                results['mongodb_time'].append(None)
                
            logger.info(f"Average write time for {size} records: "
                       f"Our DB: {avg_our_db:.4f}s, "
                       f"MongoDB: {results['mongodb_time'][-1]:.4f if results['mongodb_time'][-1] else 'N/A'}s")
            
        # Create a dataframe and generate a plot
        self._generate_performance_plot(results, "Write Performance Comparison", "write_perf")

    def test_read_performance(self):
        """Test the read performance of both databases"""
        results = {'dataset_size': [], 'our_db_time': [], 'mongodb_time': []}
        
        # Skip the test if either database is not available
        if self.our_db is None or (self.mongodb is None and MONGODB_AVAILABLE):
            self.skipTest("One or both databases are not available")
            
        for size in self.DATASET_SIZES:
            our_db_times = []
            mongodb_times = []
            
            for iteration in range(self.TEST_ITERATIONS):
                # Generate and store test data
                test_data = self.generate_test_data(size)
                
                # Store data in our database
                for record in test_data:
                    import json
                    self.our_db.put(record['id'], json.dumps(record))
                self.our_db.sync()
                
                # Store data in MongoDB if available
                if self.mongodb is not None and MONGODB_AVAILABLE:
                    self.mongodb_collection.insert_many(test_data)
                
                # Generate random IDs for read testing
                read_ids = [random.randint(0, size-1) for _ in range(self.READ_OPERATIONS)]
                
                # Test our database read performance
                start_time = time.time()
                for id in read_ids:
                    value = self.our_db.get(id)
                our_db_time = time.time() - start_time
                our_db_times.append(our_db_time)
                
                # Test MongoDB read performance if available
                if self.mongodb is not None and MONGODB_AVAILABLE:
                    start_time = time.time()
                    for id in read_ids:
                        record = self.mongodb_collection.find_one({'id': id})
                    mongodb_time = time.time() - start_time
                    mongodb_times.append(mongodb_time)
                    
                    # Clear collection for next iteration
                    self.mongodb_collection.delete_many({})
                else:
                    mongodb_times.append(None)
                
                logger.info(f"Read test, dataset size {size}, iteration {iteration+1}: "
                           f"Our DB: {our_db_time:.4f}s, "
                           f"MongoDB: {mongodb_times[-1]:.4f if mongodb_times[-1] else 'N/A'}s")
            
            # Calculate average times
            avg_our_db = sum(our_db_times) / len(our_db_times)
            results['dataset_size'].append(size)
            results['our_db_time'].append(avg_our_db)
            
            if self.mongodb is not None and MONGODB_AVAILABLE:
                avg_mongodb = sum(mongodb_times) / len(mongodb_times)
                results['mongodb_time'].append(avg_mongodb)
            else:
                results['mongodb_time'].append(None)
                
            logger.info(f"Average read time for {size} records ({self.READ_OPERATIONS} operations): "
                       f"Our DB: {avg_our_db:.4f}s, "
                       f"MongoDB: {results['mongodb_time'][-1]:.4f if results['mongodb_time'][-1] else 'N/A'}s")
            
        # Create a dataframe and generate a plot
        self._generate_performance_plot(results, "Read Performance Comparison", "read_perf")

    def test_range_query_performance(self):
        """Test the range query performance of both databases"""
        results = {'dataset_size': [], 'our_db_time': [], 'mongodb_time': []}
        
        # Skip the test if either database is not available
        if self.our_db is None or (self.mongodb is None and MONGODB_AVAILABLE):
            self.skipTest("One or both databases are not available")
            
        for size in self.DATASET_SIZES:
            our_db_times = []
            mongodb_times = []
            
            for iteration in range(self.TEST_ITERATIONS):
                # Generate and store test data
                test_data = self.generate_test_data(size)
                
                # Store data in our database
                for record in test_data:
                    import json
                    self.our_db.put(record['id'], json.dumps(record))
                self.our_db.sync()
                
                # Store data in MongoDB if available
                if self.mongodb is not None and MONGODB_AVAILABLE:
                    self.mongodb_collection.insert_many(test_data)
                
                # Generate random range queries
                range_queries = []
                for _ in range(50):  # 50 range queries
                    start = random.randint(0, size-100)
                    end = start + random.randint(10, 100)
                    if end >= size:
                        end = size - 1
                    range_queries.append((start, end))
                
                # Test our database range query performance
                start_time = time.time()
                for start, end in range_queries:
                    results = self.our_db.range_query(start, end)
                our_db_time = time.time() - start_time
                our_db_times.append(our_db_time)
                
                # Test MongoDB range query performance if available
                if self.mongodb is not None and MONGODB_AVAILABLE:
                    start_time = time.time()
                    for start, end in range_queries:
                        records = list(self.mongodb_collection.find({'id': {'$gte': start, '$lte': end}}))
                    mongodb_time = time.time() - start_time
                    mongodb_times.append(mongodb_time)
                    
                    # Clear collection for next iteration
                    self.mongodb_collection.delete_many({})
                else:
                    mongodb_times.append(None)
                
                logger.info(f"Range query test, dataset size {size}, iteration {iteration+1}: "
                           f"Our DB: {our_db_time:.4f}s, "
                           f"MongoDB: {mongodb_times[-1]:.4f if mongodb_times[-1] else 'N/A'}s")
            
            # Calculate average times
            avg_our_db = sum(our_db_times) / len(our_db_times)
            results['dataset_size'].append(size)
            results['our_db_time'].append(avg_our_db)
            
            if self.mongodb is not None and MONGODB_AVAILABLE:
                avg_mongodb = sum(mongodb_times) / len(mongodb_times)
                results['mongodb_time'].append(avg_mongodb)
            else:
                results['mongodb_time'].append(None)
                
            logger.info(f"Average range query time for {size} records: "
                       f"Our DB: {avg_our_db:.4f}s, "
                       f"MongoDB: {results['mongodb_time'][-1]:.4f if results['mongodb_time'][-1] else 'N/A'}s")
            
        # Create a dataframe and generate a plot
        self._generate_performance_plot(results, "Range Query Performance Comparison", "range_query_perf")

    def _generate_performance_plot(self, results: Dict[str, List[Any]], title: str, filename: str):
        """Generate a performance comparison plot
        
        Args:
            results: Dictionary containing test results
            title: Plot title
            filename: Base filename for saving the plot
        """
        try:
            df = pd.DataFrame(results)
            
            # Create a bar chart
            plt.figure(figsize=(10, 6))
            
            x = np.arange(len(df['dataset_size']))
            width = 0.35
            
            fig, ax = plt.subplots()
            rects1 = ax.bar(x - width/2, df['our_db_time'], width, label='Our Database')
            
            # Only plot MongoDB results if available
            if df['mongodb_time'].count() > 0:
                rects2 = ax.bar(x + width/2, df['mongodb_time'], width, label='MongoDB')
            
            ax.set_xlabel('Dataset Size')
            ax.set_ylabel('Time (seconds)')
            ax.set_title(title)
            ax.set_xticks(x)
            ax.set_xticklabels(df['dataset_size'])
            ax.legend()
            
            fig.tight_layout()
            
            # Save plot to results directory
            results_dir = os.path.join(os.path.dirname(__file__), '..', 'results')
            os.makedirs(results_dir, exist_ok=True)
            plot_path = os.path.join(results_dir, f"{filename}.png")
            plt.savefig(plot_path)
            logger.info(f"Performance plot saved to {plot_path}")
            
            # Also save data as CSV for future analysis
            csv_path = os.path.join(results_dir, f"{filename}.csv")
            df.to_csv(csv_path, index=False)
            logger.info(f"Performance data saved to {csv_path}")
            
        except Exception as e:
            logger.error(f"Error generating performance plot: {e}")

if __name__ == '__main__':
    unittest.main()