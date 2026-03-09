INSERT INTO chat.channel_members (channel_id, user_id, role)
VALUES ($1, $2, $3)
ON CONFLICT (channel_id, user_id) DO NOTHING;
