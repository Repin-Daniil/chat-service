DROP SCHEMA IF EXISTS chat CASCADE;

CREATE SCHEMA IF NOT EXISTS chat;

SET
    search_path TO chat;

-- ===========================
--  ENUM TYPES
-- ===========================
CREATE TYPE chat.channel_type AS ENUM ('DIRECT', 'GROUP', 'BROADCAST');

CREATE TYPE chat.member_role AS ENUM ('READER', 'WRITER', 'ADMIN', 'OWNER');

-- CREATE TYPE chat.member_status AS ENUM ('ACTIVE', 'LEFT', 'BANNED')
-- ===========================
--  FUNCTIONS
-- ===========================

CREATE OR REPLACE FUNCTION chat.set_updated_at()
RETURNS trigger AS $$
BEGIN
    IF NEW IS DISTINCT FROM OLD THEN
        NEW.updated_at = now();
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- ===========================
--  USERS
-- ===========================
CREATE TABLE chat.users (
    user_id TEXT NOT NULL PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    display_name TEXT NOT NULL,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    biography TEXT,
    is_deleted BOOLEAN NOT NULL DEFAULT false,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- ===========================
--  CHANNELS
-- ===========================
CREATE TABLE chat.channels (
    channel_id TEXT PRIMARY KEY,
    type chat.channel_type NOT NULL,
    title TEXT, -- NULL for DIRECT channel
    description TEXT, -- NULL for DIRECT channel
    is_deleted BOOLEAN NOT NULL DEFAULT false,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    -- last message time
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);


CREATE TABLE chat.channel_members (
    channel_id TEXT NOT NULL REFERENCES chat.channels(channel_id) ON DELETE CASCADE,
    user_id TEXT NOT NULL REFERENCES chat.users(user_id),
    role chat.member_role NOT NULL DEFAULT 'READER',
    -- status chat.member_status NOT NULL DEFAULT 'ACTIVE'
    joined_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    -- last_read_message_id BIGINT
    PRIMARY KEY (channel_id, user_id)
);

CREATE INDEX idx_channel_members_user_id ON chat.channel_members(user_id);

-- ===========================
--  FOR MAPPERS
-- ===========================
CREATE TYPE chat.user AS (
    user_id TEXT,
    username TEXT,
    display_name TEXT,
    password_hash TEXT,
    salt TEXT,
    biography TEXT
);

-- ===========================
--  UPDATE TRIGGERS
-- ===========================
CREATE TRIGGER users_set_updated_at BEFORE
UPDATE
    ON chat.users FOR EACH ROW EXECUTE FUNCTION chat.set_updated_at();

CREATE TRIGGER chats_set_updated_at BEFORE
UPDATE
    ON chat.channels FOR EACH ROW EXECUTE FUNCTION chat.set_updated_at();