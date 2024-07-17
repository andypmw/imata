import React, { useCallback, useState, useEffect } from 'react';
import { RefreshControl, ScrollView, StyleSheet, Text, View } from "react-native";
import { Avatar, Card, IconButton, Button } from 'react-native-paper';
import { router } from 'expo-router';
import { MaterialCommunityIcons } from '@expo/vector-icons';

export default function Index() {
  const [refreshing, setRefreshing] = React.useState(false);
  const [sessions, setSessions] = useState<string[]>([]);

  const fetchSessions = useCallback(async () => {
    try {
      const response = await fetch('http://192.168.4.1:80/sessions');
      const data = await response.json();
      setSessions(data);
    } catch (error) {
      console.error('Error fetching sessions:', error);
    }
  }, []);

  useEffect(() => {
    fetchSessions();
  }, [fetchSessions]);

  const onRefresh = React.useCallback(async () => {
    setRefreshing(true);

    await fetchSessions();

    setRefreshing(false);
  }, [fetchSessions]);

  return (
    <View style={styles.container}>
        {
          sessions.length === 0?
            <View style={styles.noSessionContainer}>
              <MaterialCommunityIcons name="sleep" size={96} color="black" />
              <Text style={styles.title}>No sessions available...</Text>
              <Text style={styles.subtitle}>Why not start capturing?</Text>

              <Button mode="text" style={styles.refreshButton} onPress={fetchSessions}>
                Refresh
              </Button>
            </View>
          :
            <View style={styles.container}>
              <ScrollView 
                style={styles.scrollView}
                refreshControl={
                  <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
                }
              >
                {sessions.map((sessionId, index) => (
                  <Card.Title
                    key={index}
                    title={sessionId}
                    left={(props) => <Avatar.Icon {...props} icon="camera" />}
                    right={(props) => <IconButton {...props} icon="play-circle" onPress={() => router.push('/detail?sessionId=' + sessionId)} />}
                  />
                ))}
              </ScrollView>
            </View>
        }
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  noSessionContainer: {
    flex: 1, 
    justifyContent: "center", 
    alignItems: "center",
  },
  title: {
    marginTop: 40, 
    fontSize: 20, 
    fontWeight: 'bold',
  },
  subtitle: {
    marginTop: 8,
  },
  refreshButton: {
    marginTop: 30,
  },
  scrollView: {
    paddingLeft: 15, 
    paddingRight: 15,
  }
});
