-- Поменял uuid на TEXT, тут этого нет
-- -- ================================
-- -- Initial Data for Users
-- -- ================================

-- INSERT INTO chat.users (username, display_name, password_hash, biography)
-- VALUES
-- ('alice', 'Alice Wonderland', 'hashed_password_1', 'Loves adventures.'),
-- ('bob', 'Bob Builder', 'hashed_password_2', 'Can we fix it? Yes, we can!'),
-- ('carol', 'Carol Singer', 'hashed_password_3', 'Music is life.'),
-- ('dave', 'Dave Programmer', 'hashed_password_4', 'Code is poetry.');


-- -- ================================
-- -- Initial Data for Chats
-- -- ================================

-- INSERT INTO chat.chats (type, owner_id, title, description, max_members)
-- VALUES
-- ('DM', NULL, NULL, NULL, NULL), -- Example DM without a title
-- ('GROUP', (SELECT user_id FROM users WHERE username='alice'), 'Adventurers', 'Group for adventure lovers', 50),
-- ('CHANNEL', (SELECT user_id FROM users WHERE username='carol'), 'Music Channel', 'All about music', NULL);


-- -- ================================
-- -- Initial Data for Chat Members
-- -- ================================

-- -- DM between Alice and Bob
-- INSERT INTO chat.chat_members (chat_id, user_id, role)
-- VALUES
-- (
--     (SELECT chat_id FROM chats WHERE type='DM' LIMIT 1),
--     (SELECT user_id FROM users WHERE username='alice'),
--     'MEMBER'
-- ),
-- (
--     (SELECT chat_id FROM chats WHERE type='DM' LIMIT 1),
--     (SELECT user_id FROM users WHERE username='bob'),
--     'MEMBER'
-- );

-- -- Group "Adventurers"
-- INSERT INTO chat.chat_members (chat_id, user_id, role)
-- VALUES
-- (
--     (SELECT chat_id FROM chats WHERE title='Adventurers'),
--     (SELECT user_id FROM users WHERE username='alice'),
--     'OWNER'
-- ),
-- (
--     (SELECT chat_id FROM chats WHERE title='Adventurers'),
--     (SELECT user_id FROM users WHERE username='bob'),
--     'MEMBER'
-- ),
-- (
--     (SELECT chat_id FROM chats WHERE title='Adventurers'),
--     (SELECT user_id FROM users WHERE username='dave'),
--     'MEMBER'
-- );

-- -- Channel "Music Channel"
-- INSERT INTO chat.chat_members (chat_id, user_id, role)
-- VALUES
-- (
--     (SELECT chat_id FROM chats WHERE title='Music Channel'),
--     (SELECT user_id FROM users WHERE username='carol'),
--     'OWNER'
-- ),
-- (
--     (SELECT chat_id FROM chats WHERE title='Music Channel'),
--     (SELECT user_id FROM users WHERE username='alice'),
--     'MEMBER'
-- );
