import React, { useState, useEffect } from 'react';
import { StyleSheet, View, Image, FlatList, ListRenderItem } from "react-native";
import { useLocalSearchParams } from 'expo-router';

const SessionDetail = () => {
  const { sessionId } = useLocalSearchParams<{ sessionId: string }>();
  const [imageUrls, setImageUrls] = useState<string[]>([]);

  useEffect(() => {
    const fetchImages = async () => {
      try {
        const response = await fetch('http://192.168.4.1/session?folder=' + sessionId);
        const data = await response.json();
        const transformedData = data.map((name: string) => `http://192.168.4.1/photo-download?name=${sessionId}/${name}`);
        setImageUrls(transformedData);
      } catch (error) {
        console.error('Error fetching images:', error);
      }
    };

    fetchImages();
  }, [sessionId]);

  // Render each image item
  const renderItem: ListRenderItem<string> = ({ item }) => (
    <View style={styles.imageContainer}>
      <Image 
        style={styles.photo}
        source={{ uri: item }}
      />
    </View>
  );

  return (
    <FlatList
      data={imageUrls}
      renderItem={renderItem}
      numColumns={1} // Adjust the number of columns as needed
      contentContainerStyle={styles.container}
    />
  );
};

const styles = StyleSheet.create({
  container: {
    padding: 4,
  },
  imageContainer: {
    flex: 1,

    marginTop: 8,
    marginLeft: 16,
    marginRight: 16,
    marginBottom: 8,

    borderWidth: 1,
    borderColor: '#888',
    borderRadius: 5,
    overflow: 'hidden',
    // Shadow properties for iOS
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.8,
    shadowRadius: 2,
    // Elevation for Android
    elevation: 5,
  },
  photo: {
    width: '100%',
    resizeMode: 'cover',
    aspectRatio: 1,
  }
});

export default SessionDetail;