import unittest
from src.python.api import DatabaseAPI

class TestQueries(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.db_api = DatabaseAPI()
        cls.db_api.connect("test_database")

    @classmethod
    def tearDownClass(cls):
        cls.db_api.disconnect()

    def test_execute_query(self):
        result = self.db_api.execute_query("SELECT * FROM test_table")
        self.assertIsNotNone(result)
        self.assertIsInstance(result, list)

    def test_insert_query(self):
        insert_result = self.db_api.execute_query("INSERT INTO test_table (column1) VALUES ('value1')")
        self.assertTrue(insert_result)

    def test_update_query(self):
        update_result = self.db_api.execute_query("UPDATE test_table SET column1 = 'updated_value' WHERE column1 = 'value1'")
        self.assertTrue(update_result)

    def test_delete_query(self):
        delete_result = self.db_api.execute_query("DELETE FROM test_table WHERE column1 = 'updated_value'")
        self.assertTrue(delete_result)

if __name__ == '__main__':
    unittest.main()