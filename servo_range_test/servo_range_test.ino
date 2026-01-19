/*
 * SG90 Servo Motor Range Test
 * এই কোড দিয়ে আপনার সার্ভো মোটরের নিরাপদ রেঞ্জ টেস্ট করুন
 *
 * সংযোগ:
 * - Servo VCC (লাল) → ESP32 5V
 * - Servo GND (বাদামী/কালো) → ESP32 GND
 * - Servo Signal (কমলা/হলুদ) → ESP32 GPIO 13
 */

#include <ESP32Servo.h>

Servo testServo;
const int SERVO_PIN = 13;

// নিরাপদ সীমা
const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 160;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println("SG90 Servo Motor Range Test");
  Serial.println("=================================");
  Serial.println();

  testServo.attach(SERVO_PIN);

  // প্রাথমিক পজিশন
  testServo.write(90);
  Serial.println("✓ Servo initialized at 90°");
  delay(2000);
}

void loop() {
  Serial.println("\n--- Test Cycle Started ---");

  // টেস্ট 1: মিনিমাম এঙ্গেল
  Serial.print("Moving to MINIMUM (");
  Serial.print(SERVO_MIN_ANGLE);
  Serial.println("°)...");
  testServo.write(SERVO_MIN_ANGLE);
  delay(1500);

  // টেস্ট 2: সেন্টার
  Serial.println("Moving to CENTER (90°)...");
  testServo.write(90);
  delay(1500);

  // টেস্ট 3: ম্যাক্সিমাম এঙ্গেল (নিরাপদ)
  Serial.print("Moving to MAXIMUM (");
  Serial.print(SERVO_MAX_ANGLE);
  Serial.println("°)...");
  testServo.write(SERVO_MAX_ANGLE);
  delay(1500);

  // টেস্ট 4: সেন্টারে ফিরে আসা
  Serial.println("Returning to CENTER (90°)...");
  testServo.write(90);
  delay(1500);

  // টেস্ট 5: মসৃণ স্ক্যান (0° থেকে 160°)
  Serial.println("\nSmooth Scan Test (0° → 160°)...");
  for (int angle = SERVO_MIN_ANGLE; angle <= SERVO_MAX_ANGLE; angle += 10) {
    testServo.write(angle);
    Serial.print(angle);
    Serial.print("° ");
    delay(200);
  }
  Serial.println("\n✓ Scan complete");

  // টেস্ট 6: রিভার্স স্ক্যান (160° থেকে 0°)
  Serial.println("Reverse Scan Test (160° → 0°)...");
  for (int angle = SERVO_MAX_ANGLE; angle >= SERVO_MIN_ANGLE; angle -= 10) {
    testServo.write(angle);
    Serial.print(angle);
    Serial.print("° ");
    delay(200);
  }
  Serial.println("\n✓ Reverse scan complete");

  // সেন্টারে ফিরে আসা
  testServo.write(90);

  Serial.println("\n=== Test Cycle Complete ===");
  Serial.println("Waiting 3 seconds before next cycle...\n");
  delay(3000);
}

/*
 * পর্যবেক্ষণ করুন:
 * ✓ মোটর মসৃণভাবে ঘুরছে কিনা
 * ✓ কোনো বিপ/গুঁজগুঁজ শব্দ হচ্ছে কিনা
 * ✓ কোনো এঙ্গেলে আটকে যাচ্ছে কিনা
 *
 * যদি সমস্যা হয়:
 * - SERVO_MAX_ANGLE আরও কমিয়ে 150° করুন
 * - পাওয়ার সাপ্লাই চেক করুন
 * - মোটরের সংযোগ চেক করুন
 */
