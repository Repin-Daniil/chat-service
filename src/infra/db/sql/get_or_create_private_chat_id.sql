WITH inserted_channel AS (
    INSERT INTO chat.channels (channel_id, type)
    VALUES ($1, 'DIRECT')
    ON CONFLICT (channel_id) DO NOTHING
    RETURNING channel_id
),
inserted_members AS (

    INSERT INTO chat.channel_members (channel_id, user_id, role)
    VALUES 
        ($1, $2, 'OWNER'),
        ($1, $3, 'OWNER')
    ON CONFLICT (channel_id, user_id) DO NOTHING
)

SELECT 
    channel_id, 
    TRUE AS is_new 
FROM inserted_channel

UNION ALL

SELECT 
    channel_id, 
    FALSE AS is_new 
FROM chat.channels 
WHERE channel_id = $1 
  AND NOT EXISTS (SELECT 1 FROM inserted_channel);