DROP SCHEMA IF EXISTS chat CASCADE;

CREATE SCHEMA IF NOT EXISTS chat;

SET search_path TO chat;

-- ===========================
--  ENUM TYPES
-- ===========================

CREATE TYPE chat.chat_type AS ENUM (
    'DM',
    'GROUP',
    'CHANNEL'
);

CREATE TYPE chat.member_role AS ENUM (
    'MEMBER',
    'ADMIN',
    'OWNER'
);

-- ===========================
--  USERS
-- ===========================

CREATE TABLE chat.users (
    user_id       TEXT NOT NULL PRIMARY KEY,
    username      TEXT NOT NULL UNIQUE,
    display_name  TEXT NOT NULL,
    password_hash TEXT NOT NULL,
    salt          TEXT NOT NULL,
    biography     TEXT,
    is_deleted    BOOLEAN NOT NULL DEFAULT false,
    created_at    TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at    TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- ===========================
--  CHATS
-- ===========================

CREATE TABLE chat.chats (
    chat_id       TEXT NOT NULL PRIMARY KEY,
    type          chat.chat_type NOT NULL,
    owner_id      TEXT REFERENCES chat.users(user_id) ON DELETE SET NULL,
    title         TEXT,
    description   TEXT,
    is_deleted    BOOLEAN NOT NULL DEFAULT false,
    max_members   INT,
    created_at    TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at    TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX chats_type_idx ON chat.chats(type);

-- ===========================
--  CHAT MEMBERS
-- ===========================

CREATE TABLE chat.chat_members (
    chat_id   TEXT NOT NULL REFERENCES chat.chats(chat_id) ON DELETE CASCADE,
    user_id   TEXT NOT NULL REFERENCES chat.users(user_id) ON DELETE CASCADE,
    role      chat.member_role NOT NULL DEFAULT 'MEMBER',
    joined_at TIMESTAMPTZ NOT NULL DEFAULT now(),

    PRIMARY KEY (chat_id, user_id)
);

-- ===========================
--  (OPTIONAL) CHAT BANS
-- ===========================

-- CREATE TABLE chat.chat_bans (
--     chat_id       TEXT NOT NULL REFERENCES chat.chats(chat_id) ON DELETE CASCADE,
--     user_id       TEXT NOT NULL REFERENCES chat.users(user_id) ON DELETE CASCADE,
--     banned_by     TEXT NOT NULL REFERENCES chat.users(user_id) ON DELETE SET NULL,
--     banned_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
--     reason        TEXT,
--
--     PRIMARY KEY (chat_id, user_id)
-- );


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