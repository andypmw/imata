import { useState } from "react";
import { StyleSheet, View, Text } from "react-native";
import { Button, SegmentedButtons } from "react-native-paper";

const Settings = () => {
  const [maxPhotosPerSession, setMaxPhotosPerSession] = useState(60);
  const [captureInterval, setCaptureInterval] = useState(1);

  return (
    <View
      style={{
        flex: 1,
        justifyContent: "center",
        padding: 20,
      }}
    >
      <Text style={styles.label}>Max photos per session</Text>
      <SegmentedButtons
        style={styles.configOptions}
        value={maxPhotosPerSession.toString()}
        onValueChange={(val) => setMaxPhotosPerSession(Number(val))}
        buttons={[
          {
            value: '60',
            label: '60',
          },
          {
            value: '300',
            label: '300',
          },
          {
            value: '1500',
            label: '1500',
          },
        ]}
      />

      <Text style={styles.label}>Capture interval</Text>
      <SegmentedButtons
        value={captureInterval.toString()}
        onValueChange={(val) => setCaptureInterval(Number(val))}
        buttons={[
          {
            value: '1',
            label: '1',
          },
          {
            value: '5',
            label: '5',
          },
          {
            value: '15',
            label: '15',
          },
          {
            value: '30',
            label: '30',
          },
        ]}
      />

      <Button icon="content-save-cog" mode="contained" onPress={() => console.log('Pressed')} style={{marginTop: 50}}>
        Save
      </Button>
    </View>
  );
};

const styles = StyleSheet.create({
  label: {
    fontSize: 16,
    marginBottom: 4,
  },
  configOptions: {
    marginBottom: 20,
  }
});

export default Settings;