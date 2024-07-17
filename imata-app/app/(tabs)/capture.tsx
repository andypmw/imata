import { useState, useEffect, ReactElement } from 'react';
import { StyleSheet, View, Text } from "react-native";
import { FontAwesome } from '@expo/vector-icons';
import { FontAwesome5 } from '@expo/vector-icons';
import { FontAwesome6 } from '@expo/vector-icons';
import { Button } from 'react-native-paper';
import { MaterialCommunityIcons } from '@expo/vector-icons';

const Capture = () => {
  const icons: ReactElement[] = [
    <FontAwesome5 name="eye" size={96} color="black" />,
    <FontAwesome name="photo" size={96} color="black" />,
    <FontAwesome5 name="smile-wink" size={96} color="black" />,
    <FontAwesome6 name="folder-open" size={96} color="black" />,
  ];

  const [currentIconIndex, setCurrentIconIndex] = useState<number>(0);
  const [status, setStatus] = useState<string>('idle');
  const [sessionId, setSessionId] = useState<string>('');
 
  useEffect(() => {
    const intervalId = setInterval(() => {
      setCurrentIconIndex((prevIndex) => (prevIndex + 1) % icons.length);
    }, 800);

    return () => clearInterval(intervalId);
  }, [icons.length]);

  const toggleCapture = async () => {
    if (status === 'idle') {
      try {
        // First request to start/stop capture
        await fetch('http://192.168.4.1/start-stop-capture', {
          method: 'POST',
        });
  
        // Second request to get capture status
        const response = await fetch('http://192.168.4.1/capture-status', {
          method: 'GET',
        });
        const captureStatus = await response.json();
        
        if (captureStatus.message === "Capture in progress.") {
          setStatus('capturing');
          setSessionId(captureStatus.sessionFolderName);
        }
      } catch (error) {
        console.error('Error:', error);
      }
    } else {
      try {
        // First request to start/stop capture
        await fetch('http://192.168.4.1/start-stop-capture', {
          method: 'POST',
        });
  
        // Second request to get capture status
        const response = await fetch('http://192.168.4.1/capture-status', {
          method: 'GET',
        });
        const captureStatus = await response.json();
        
        if (captureStatus.message === "No capture in progress.") {
          setStatus('idle');
          setSessionId('');
        }
      } catch (error) {
        console.error('Error:', error);
      }
    }
  }

  return (
    <View style={styles.container}>
      <View style={styles.iconContainer}>
        {status === 'idle'?
          <MaterialCommunityIcons name="sleep" size={96} color="black" />
        :
          icons[currentIconIndex]
        }
      </View>

      <Text style={styles.statusTitle}>{status === 'idle' ? 'Sleeping' : 'Capturing'}</Text>
      <Text style={styles.statusSubtitle}>{status === 'idle' ? '' : 'Session ID: ' + sessionId}</Text>

      <Button icon={status === 'idle' ? 'camera' : 'hand-back-left'} mode="contained" onPress={() => toggleCapture()} style={styles.controlButton}>
        {status === 'idle' ? 'Start' : 'Stop'}
      </Button>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
  },
  iconContainer: {
    height: 120,
  },
  statusTitle: {
    fontSize: 40,
  },
  statusSubtitle: {
    marginTop: 40,
  },
  controlButton: {
    marginTop: 50,
  }
});

export default Capture;