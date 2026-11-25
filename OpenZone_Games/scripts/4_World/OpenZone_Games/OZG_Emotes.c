// Two doors into the game's gesture manager for the poses at a game
// (5_Mission, OZG_PoseDriver): which gesture is on now, and its end. Both are
// protected in the game (emotemanager.c:148, m_CurrentGestureID).
modded class EmoteManager
{
    int OZG_CurrentGesture()
    {
        return m_CurrentGestureID;
    }

    // A request sent to the server and not answered yet.
    bool OZG_Pending()
    {
        return m_bEmoteIsRequestPending;
    }

    // The gesture on now ends, the way a step of the player ends it.
    void OZG_EndGesture()
    {
        SendEmoteRequestSync(CALLBACK_CMD_END);
    }
}
