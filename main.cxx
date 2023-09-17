#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

class IDBConnection {
public:
    IDBConnection() { }
    virtual ~IDBConnection() { }

    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool execQuery(std::string const& query) = 0;
};

// inherit from the interface
class DBConnection : public IDBConnection {
public:
    DBConnection() { }
    void open() override
    {
        if (!is_open)
            return;
        is_open = true;
    }
    bool execQuery(std::string const& query) override
    {
        return true; // the query is successful
    }
    void close() override { is_open = false; }

protected:
    bool is_open = true;
};

// Mock class
class MockDBConnection : virtual public IDBConnection {
public:
    MOCK_METHOD(void, open, (), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, execQuery, (std::string const& query), (override));
};

class ClassThatUsesDB {
public:
    ClassThatUsesDB(IDBConnection* db_connnection)
        : connection_(db_connnection)
    {
    }

    void openConnection() { connection_->open(); }
    bool useConnection(std::string const& query)
    {
        return connection_->execQuery(query);
    }
    void closeConnection() { connection_->close(); }

private:
    IDBConnection* connection_;
};

class DBTestSuite : public ::testing::Test {
protected:
    void SetUp()
    {
        conn_db_ = new DBConnection();
        conn_user_ = new ClassThatUsesDB(conn_db_);
    }

    void TearDown()
    {
        delete conn_db_;
        delete conn_user_;
    }

protected:
    IDBConnection* conn_db_;
    ClassThatUsesDB* conn_user_;
};

TEST_F(DBTestSuite, test_user_query)
{
    conn_user_->openConnection();
    bool res = conn_user_->useConnection("SELECT username FROM users;");
    ASSERT_EQ(res, true);
    conn_user_->closeConnection();
}

TEST_F(DBTestSuite, test_db_query)
{

    conn_db_->open();
    bool res = conn_db_->execQuery("SELECT username FROM users;");
    ASSERT_EQ(res, true);
    conn_db_->close();
}

TEST_F(DBTestSuite, test_db_open) { conn_db_->open(); }

TEST_F(DBTestSuite, test_db_open_and_close)
{
    conn_db_->open();
    conn_db_->close();
}

TEST_F(DBTestSuite, test_mock_db)
{
    MockDBConnection mock_db;
    EXPECT_CALL(mock_db, execQuery("CREATE TABLE users;"))
        .WillOnce(::testing::Return(true));
    ClassThatUsesDB mock_uses_db(&mock_db);
    mock_uses_db.openConnection();
    bool res = mock_uses_db.useConnection("CREATE TABLE users;");
    mock_uses_db.closeConnection();
    ASSERT_EQ(res, true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
