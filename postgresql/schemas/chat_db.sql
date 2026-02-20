DROP SCHEMA IF EXISTS chat CASCADE;

CREATE SCHEMA IF NOT EXISTS chat;

SET
    search_path TO chat;

-- ===========================
--  ENUM TYPES
-- ===========================
CREATE TYPE chat.chat_type AS ENUM ('PRIVATE', 'GROUP', 'CHANNEL');
-- todo в базе не должно быть по идее никаких каналов, это та же группа, просто by default участник reader
CREATE TYPE chat.member_role AS ENUM ('MEMBER', 'ADMIN', 'OWNER');

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
--  CHATS
-- ===========================
CREATE TABLE chat.chats (
    chat_id TEXT PRIMARY KEY,
    type chat.chat_type NOT NULL,
    is_deleted BOOLEAN NOT NULL DEFAULT false,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE chat.groups (
    chat_id TEXT PRIMARY KEY REFERENCES chat.chats(chat_id) ON DELETE CASCADE,
    title TEXT NOT NULL,
    description TEXT,
    owner_id TEXT NOT NULL REFERENCES chat.users(user_id),
    max_members INT DEFAULT 200000,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE chat.private_chats (
    chat_id TEXT PRIMARY KEY REFERENCES chat.chats(chat_id) ON DELETE CASCADE,
    user_id_1 TEXT NOT NULL,
    user_id_2 TEXT NOT NULL,
    CHECK (user_id_1 <= user_id_2),
    UNIQUE (user_id_1, user_id_2)
);

CREATE TABLE chat.group_members (
    chat_id TEXT NOT NULL REFERENCES chat.groups(chat_id) ON DELETE CASCADE,
    user_id TEXT NOT NULL REFERENCES chat.users(user_id),
    role chat.member_role NOT NULL DEFAULT 'MEMBER',
    joined_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (chat_id, user_id)
);
-- todo нужен обратный индекс user_id, chat_id для формирования чатов пользователя

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
    ON chat.chats FOR EACH ROW EXECUTE FUNCTION chat.set_updated_at();