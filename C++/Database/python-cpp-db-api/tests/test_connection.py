import unittest
from src.python.connection import DatabaseConnection

class TestDatabaseConnection(unittest.TestCase):

    def setUp(self):
        self.db_connection = DatabaseConnection()

    def test_connection_success(self):
        result = self.db_connection.connect("test_db", "user", "password")
        self.assertTrue(result)

    def test_connection_failure(self):
        result = self.db_connection.connect("invalid_db", "user", "wrong_password")
        self.assertFalse(result)

    def tearDown(self):
        self.db_connection.disconnect()

if __name__ == '__main__':
    unittest.main()